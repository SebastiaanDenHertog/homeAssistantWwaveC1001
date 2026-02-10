/**
 * @Authors:            Laurens Leusink, Sebastiaan den Hertog, Tom Keuper
 * @Date created:       10-10-2024
 * @Date updated:       26-11-2024 (By: Tom Keuper)
 * @Description:        Class for handling the ethernet of the ESP32.
 */

#include "Internet.h"
#include "C1001Controller.h"

Preferences preferences;

bool Internet::eth_connected = false;

Internet::Internet() : useTcp(false), tcpServer(nullptr), dmxCallback(nullptr) {}

/**
 * @brief Initializes the Ethernet connection. The hostname and AP name are automatically set with a combination of "Sphere-" and the last 5 characters of the MAC address.
 */
void Internet::begin() {
    ETH.begin(phy_addr, power, mdc, mdio, type, clock_mode);

    String name = getControllerName();

    ETH.setHostname(name.c_str());
    WiFi.softAP(name, "Controller123!"); // Start the AP

    DEBUG_PRINT("IP Address of webpage: ");
    DEBUG_PRINTLN(WiFi.softAPIP());

    applySettings();
    WiFi.onEvent(WiFiEvent);
}

/**
 * @brief Handles the WiFi events by printing the event that has occurred
 * @param event The event that has occurred
 */
void Internet::WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            DEBUG_PRINTLN("ETH Started");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            DEBUG_PRINTLN("ETH Connected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            DEBUG_PRINT("ETH MAC: ");
            DEBUG_PRINT(ETH.macAddress());
            DEBUG_PRINT(", IPv4: ");
            DEBUG_PRINT(ETH.localIP());
            if (ETH.fullDuplex()) {
                DEBUG_PRINT(", FULL_DUPLEX");
            }
            DEBUG_PRINT(", ");
            DEBUG_PRINT(ETH.linkSpeed());
            DEBUG_PRINTLN("Mbps");

            eth_connected = true;
            delay(1000);
            // Auto-register with orchestrator
            autoRegisterWithOrchestrator();
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            DEBUG_PRINTLN("ETH Disconnected");
            eth_connected = false;
            break;
        case ARDUINO_EVENT_ETH_STOP:
            DEBUG_PRINTLN("ETH Stopped");
            eth_connected = false;
            break;
        default:
            break;
    }
}

/**
 * @brief Gets the settings from the preferences and stores them in the struct
 */
void Internet::getSettings() {
    byte emptyArray[4] = {0, 0, 0, 0};

    byte ip[4] = {};
    byte gateway[4] = {};
    byte subnet[4] = {};
    String hostname = "";

    preferences.begin("network", false);
    preferences.getBytes("ip", ip, sizeof(ip));
    preferences.getBytes("gateway", gateway, sizeof(gateway));
    preferences.getBytes("subnet", subnet, sizeof(subnet));
    hostname = preferences.getString("hostname");
    preferences.end();

      // Check if the settings are not empty and set them to the struct
    if (ip != emptyArray) {
        networkSettings.ip = IPAddress(ip[0], ip[1], ip[2], ip[3]);
    }
    if (gateway != emptyArray) {
        networkSettings.gateway = IPAddress(gateway[0], gateway[1], gateway[2], gateway[3]);
    }

    if (subnet != emptyArray) {
        networkSettings.subnet = IPAddress(subnet[0], subnet[1], subnet[2], subnet[3]);
    }

    if (hostname != "") {
        networkSettings.hostname = hostname.c_str();
    }
    else {
        networkSettings.hostname = getControllerName();
    }
}

/**
 * @brief Applies the settings to the Ethernet connection
 */
void Internet::applySettings(){
    getSettings();

    // Uses DHCP if the IP address is not set
    if (networkSettings.ip == IPAddress(0, 0, 0, 0)) {
        DEBUG_PRINTLN("Using DHCP for IP configuration");
    } else {
        DEBUG_PRINTLN("Using static IP configuration");
        ETH.config(networkSettings.ip, networkSettings.gateway, networkSettings.subnet);
    }

    ETH.setHostname(networkSettings.hostname.c_str());

}
/**
 * @brief Updates the settings in the preferences by getting the data from the struct. And updating the Ethernet connection
 */
void Internet::updateSettings() {
    byte ip[4] = {networkSettings.ip[0], networkSettings.ip[1], networkSettings.ip[2], networkSettings.ip[3]};
    byte gateway[4] = {networkSettings.gateway[0], networkSettings.gateway[1], networkSettings.gateway[2], networkSettings.gateway[3]};
    byte subnet[4] = {networkSettings.subnet[0], networkSettings.subnet[1], networkSettings.subnet[2], networkSettings.subnet[3]};

    // Store the settings in the preferences
    preferences.begin("network", false);
    preferences.putBytes("ip", (byte*)(&ip), sizeof(ip));
    preferences.putBytes("gateway", (byte*)(&gateway), sizeof(gateway));
    preferences.putBytes("subnet", (byte*)(&subnet), sizeof(subnet));
    preferences.putString("hostname", networkSettings.hostname);
    preferences.end();

    // Apply the settings to the Ethernet connection
    ETH.config(networkSettings.ip, networkSettings.gateway, networkSettings.subnet);
    ETH.setHostname(networkSettings.hostname.c_str());
}

/**
 * @brief Resets the settings in the preferences and sets the Ethernet connection to default settings
 */
void Internet::resetSettings(){
    preferences.begin("network", false);
    preferences.clear();
    preferences.putString("hostname", getControllerName());
    preferences.end();

    networkSettings.ip = IPAddress(0, 0, 0, 0);
    networkSettings.gateway = IPAddress(0, 0, 0, 0);
    networkSettings.subnet = IPAddress(0, 0, 0, 0);
    networkSettings.hostname = getControllerName(); 

    ETH.config(networkSettings.ip, networkSettings.gateway, networkSettings.subnet);
    ETH.setHostname(networkSettings.hostname.c_str());
    applySettings();
}

/**
 * @brief Displays the settings of the Ethernet connection for each storage method.
 * For Debugging purposes only
 */
void Internet::displaySettings(){
    DEBUG_PRINTLN("=====Displaying settings=====");
    DEBUG_PRINTLN("Settings in struct:");
    DEBUG_PRINT("IP: ");
    DEBUG_PRINTLN(networkSettings.ip);
    DEBUG_PRINT("Gateway: ");
    DEBUG_PRINTLN(networkSettings.gateway);
    DEBUG_PRINT("Hostname: ");
    DEBUG_PRINTLN(networkSettings.hostname);
    DEBUG_PRINT("Subnet: ");
    DEBUG_PRINTLN(networkSettings.subnet);

    DEBUG_PRINTLN("\nSettings in preferences:");
    byte ip[4] = {};
    byte gateway[4] = {};
    byte subnet[4] = {};
    String hostname = "";

    preferences.begin("network", false);
    preferences.getBytes("ip", ip, sizeof(ip));
    preferences.getBytes("gateway", gateway, sizeof(gateway));
    preferences.getBytes("subnet", subnet, sizeof(subnet));
    hostname = preferences.getString("hostname");
    preferences.end();
    
    DEBUG_PRINT("IP: ");
    DEBUG_PRINTLN(IPAddress(ip[0], ip[1], ip[2], ip[3]));
    DEBUG_PRINT("Gateway: ");
    DEBUG_PRINTLN(IPAddress(gateway[0], gateway[1], gateway[2], gateway[3]));
    DEBUG_PRINT("Hostname: ");
    DEBUG_PRINTLN(hostname);
    DEBUG_PRINT("Subnet: ");
    DEBUG_PRINTLN(IPAddress(subnet[0], subnet[1], subnet[2], subnet[3]));
    
    DEBUG_PRINTLN("\nSettings in ETH library:");
    DEBUG_PRINT("IP: ");
    DEBUG_PRINTLN(ETH.localIP());
    DEBUG_PRINT("Gateway: ");
    DEBUG_PRINTLN(ETH.gatewayIP());
    DEBUG_PRINT("Hostname: ");
    DEBUG_PRINTLN(ETH.getHostname());
    DEBUG_PRINT("Subnet: ");
    DEBUG_PRINTLN(ETH.subnetMask());

    DEBUG_PRINTLN("=====End of settings=====");
}

/**
 * @brief Sets the IP address of the Ethernet connection
 * @param ip The IP address to set
 */
void Internet::setIP(IPAddress ip) {
    networkSettings.ip = ip;
    updateSettings();
}

/**
 * @brief Sets the gateway of the Ethernet connection
 * @param gateway The gateway to set
 */
void Internet::setGateway(IPAddress gateway) {
    networkSettings.gateway = gateway;
    updateSettings();
}

/**
 * @brief Sets the subnet of the Ethernet connection
 * @param subnet The subnet to set
 */
void Internet::setSubnet(IPAddress subnet) {
    networkSettings.subnet = subnet;
    updateSettings();
}

/**
 * @brief Sets the hostname of the Ethernet connection
 * @param hostname The hostname to set
 */
void Internet::setHostname(String hostname) {
    networkSettings.hostname = hostname;
    updateSettings();
}

/**
 * @brief Gets the IP address of the Ethernet connection
 * @return The IP address of the Ethernet connection
 */
IPAddress Internet::getIP() {
    return ETH.localIP();
}

/**
 * @brief Gets the gateway of the Ethernet connection
 * @return The gateway of the Ethernet connection
 */
IPAddress Internet::getGateway() {
    return ETH.gatewayIP();
}

/**
 * @brief Gets the subnet of the Ethernet connection
 * @return The subnet of the Ethernet connection
 */
IPAddress Internet::getSubnet(){
    return ETH.subnetMask();
}

/**
 * @brief Gets the hostname of the Ethernet connection
 * @return The hostname of the Ethernet connection
 */
const char* Internet::getHostname() {
    return ETH.getHostname();
}

/**
 * @brief Gets the MAC address of the Ethernet connection
 * @return The MAC address of the Ethernet connection
 */
String Internet::getMAC() {
    return ETH.macAddress();
}

/**
 * @brief Gets the link status of the Ethernet connection
 * @return The link status of the Ethernet connection
 */
bool Internet::getLinkStatus() {
    return ETH.linkUp();
}

/**
 * @brief Gets the link speed of the Ethernet connection
 * @return The link speed of the Ethernet connection
 */
uint8_t Internet::getLinkSpeed() {
    return ETH.linkSpeed();
}

/**
 * @brief Gets the controller name based on the MAC address
 */
String Internet::getControllerName(){
    String mac = getMAC();
    String controllerName = "Controller-" + mac.substring(mac.length() - 5);
    return controllerName;
}

/**
 * @brief Registers the controller with the orchestrator by checking if it exists first
 */
void Internet::autoRegisterWithOrchestrator() {
    const String orchestratorUrl = "http://orchestrator.sphere"; // Base URL for Orchestrator
    const String macAddress = WiFi.macAddress();                // Unique identifier for this ESP32
    const String checkUrl = orchestratorUrl + "/settings/controller/exists/" + macAddress;

    WiFiClient client; // Secure client for HTTPS

    HTTPClient http;

    DEBUG_PRINTF("URL: %s\n", checkUrl.c_str());

    // Step 1: Check if controller exists
    DEBUG_PRINTLN("Checking if controller is already registered...");
    if (http.begin(client, checkUrl)) { // Start connection to the endpoint
        int httpResponseCode = http.GET(); // Send GET request


        if (httpResponseCode == HTTP_CODE_OK) { // 200 OK
            DEBUG_PRINTLN("Controller is already registered.");
            http.end();
            updateControllerData();
            return; // Exit if the controller is already registered
        } else if (httpResponseCode == HTTP_CODE_NOT_FOUND) { // 404 Not Found
            DEBUG_PRINTLN("Controller not registered. Proceeding to register...");
        } else {
            DEBUG_PRINTF("Failed to check registration. HTTP Code: %d\n", httpResponseCode);
        }
        http.end(); // Close connection
    } else {
        DEBUG_PRINTLN("Failed to begin HTTPS connection for registration check.");
    }

    // Step 2: Register the controller
    const String registerUrl = orchestratorUrl + "/settings/addController";
    DEBUG_PRINTLN("Registering controller with orchestrator...");
    if (http.begin(client, registerUrl)) {
        http.addHeader("Content-Type", "application/json"); // Always include content type for JSON

        String jsonBody = createRegistrationPayload(); // Call the function to create the registration payload
        int httpResponseCode = http.POST(jsonBody); // Send POST request with the JSON payload

        if (httpResponseCode == HTTP_CODE_OK) { // Check if registration was successful
            String response = http.getString();
            DEBUG_PRINTLN("Controller registered successfully: " + response);
        } else {
            DEBUG_PRINTF("Failed to register controller. HTTP Code: %d\n", httpResponseCode);
        }
        http.end(); // Close connection
    } else {
        DEBUG_PRINTLN("Failed to begin HTTPS connection for registering.");
    }
}
void Internet::updateControllerData() {
    // Construct the URL for the update endpoint
    String macAddress = WiFi.macAddress().c_str(); // Get the MAC address dynamically
    String url = "http://orchestrator.sphere/settings/controller/update/" + macAddress;

    // Construct the request body
    String controllerName = ETH.getHostname(); // Get the dynamic controller name
    int totalLedCount = 0;
    String segmentsJson = getSegmentsJson(totalLedCount); // Retrieve LED segments data

    // Create the JSON request body
    String requestBody = "{";
    requestBody += "\"ipAddress\":\"" + ETH.localIP().toString() + "\",";
    requestBody += "\"name\":\"" + controllerName + "\",";
    requestBody += "\"ledCount\":" + String(totalLedCount) + ",";
    requestBody += "\"segments\":" + segmentsJson;
    requestBody += "}";

    // Print the payload for debugging
    DEBUG_PRINTLN("Update Payload: " + requestBody);

    // Use HTTP client to send PUT request
    HTTPClient http;
    http.begin(url);                       // Set the destination URL
    http.addHeader("Content-Type", "application/json"); // Set the content type

    int httpResponseCode = http.PUT(requestBody); // Send the PUT request
    if (httpResponseCode > 0) {
        // Handle successful response
        String response = http.getString();
        DEBUG_PRINTLN("Response Code: " + String(httpResponseCode));
        DEBUG_PRINTLN("Response: " + response);
    } else {
        // Handle request failure
        DEBUG_PRINTLN("Error in sending PUT request: " + String(http.errorToString(httpResponseCode).c_str()));
    }

    http.end(); // Free resources
}

/**
 * @brief Creates the JSON payload for registration with the orchestrator
 * @return The JSON payload for registration
 */
String Internet::createRegistrationPayload() {
    // Calculate the total LED count and get the segments JSON
    int totalLedCount = 0;
    String segmentsJson = getSegmentsJson(totalLedCount); // Helper function to extract segments data
    String controllerName = ETH.getHostname();

    // Construct JSON payload
    String payload = "{";
    payload += "\"macAddress\":\"" + WiFi.macAddress() + "\",";
    payload += "\"ipAddress\":\"" + ETH.localIP().toString() + "\",";
    payload += "\"name\":\"" + controllerName + "\",";
    payload += "\"ledCount\":" + String(totalLedCount) + ",";
    payload += "\"segments\": " + segmentsJson;
    payload += "}";

    return payload;
}

String Internet::getSegmentsJson(int &totalLedCount) {
    JsonDocument jsonDocument; // Adjust size based on the expected number of segments
    JsonArray segmentsArray = jsonDocument.createNestedArray("segments");

    totalLedCount = 0; // Initialize total LED count

    // Loop through all buses/segments
    for (int i = 0; i < busses.getNumPins(); i++) {
        MyNamespace::Bus *bus = busses.getBus(i);

        // Skip invalid or empty buses
        if (!bus || bus->getLength() == 0) {
            continue;
        }

        JsonObject segment = segmentsArray.createNestedObject();

        // Populate the segment data
        segment["busID"] = i;
        segment["startLed"] = bus->getStart();
        segment["length"] = bus->getLength();

        // Add the pins used for this segment
        JsonArray pinsArray = segment.createNestedArray("pin");
        uint8_t pins[5]; // Assuming a maximum of 5 pins
        uint8_t pinCount = bus->getPins(pins);

        for (uint8_t p = 0; p < pinCount; p++) {
            pinsArray.add(pins[p]);
        }

        segment["rev"] = bus->isReversed();
        segment["type"] = bus->getType() & 0x7F;

        // Accumulate the total LED count
        totalLedCount += bus->getLength();
    }

    // Serialize segments as JSON string
    String segmentsJson;
    serializeJson(segmentsArray, segmentsJson);

    return segmentsJson; // Return JSON array as a string
}
