/**
 * @Authors:            Laurens Leusink, Tom Keuper
 * @Date created:       16-09-2024
 * @Date updated:       26-11-2024 (By: Tom Keuper)
 * @Description:        Class for a web server on the ESP32. The webserver can be connected to over an access point and can be used to test the system.
 */

#include "C1001Controller.h"

extern Internet internet;

WebConfigServer::WebConfigServer() : server(80) {}

/**
 * @brief sets up the webserver
 */
void WebConfigServer::begin() {
    // Setup routes
    setupRoutes();
    updateNetwork();
    resetNetworkSettings();
    GetNetworkSettings();
    setLedData();
    getLedData();

    // Starts server
    server.begin();
}

String WebConfigServer::processor(const String& var) {
    String text;
    if(var == "CHIP_ID"){
        text = String(ESP.getEfuseMac(), HEX);
    }
    else if(var == "FLASH_CHIP_SIZE"){
        text = String(ESP.getFlashChipSize() / 1024 / 1024);
    }
    else if(var == "FLASH_CHIP_SPEED"){
        text = String(ESP.getFlashChipSpeed() / 1000000);
    }
    else if(var == "SKETCH_SIZE"){
        text = String(ESP.getSketchSize() / 1024);
    }
    else if(var == "FREE_HEAP"){
        text = String(ESP.getFreeHeap() / 1024);
    }
    else if(var == "FREE_SKETCH_SPACE"){
        text = String(ESP.getFreeSketchSpace() / 1024);
    }
    else if(var == "HOSTNAME"){
        text = internet.getHostname();
    }
    else if(var == "IP_ADDRESS"){
        text = internet.getIP().toString();
    }
    else if(var == "GATEWAY"){
        text = internet.getGateway().toString();
    }
    else if(var == "SUBNET"){
        text = internet.getSubnet().toString();
    }
    else if(var == "MAC_ADDRESS"){
        text = internet.getMAC();
    }
    else if(var == "LINK_STATUS"){
        text = internet.getLinkStatus() ? "Connected" : "Disconnected";
    }
    else if(var == "LINK_SPEED"){
        text = String(internet.getLinkSpeed());
    }
    else{
        text = "Text not found";
    }

    return text;
}

/**
 * @brief sets up the homepage route
 */
void WebConfigServer::setupRoutes()
{
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route for the api.js
    server.on("/api.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/api.js", "text/javascript");
    });

    // Route for the style.css
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/style.css", "text/css");
     });

    // Route for bootstrap css
    server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/bootstrap.min.css", "text/css");
    });

    // Route for ledstrip.js
    server.on("/ledstrip.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/ledstrip.js", "text/javascript");
    });

    // Route for createHTML.js
    server.on("/createHTML.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/createHTML.js", "text/javascript");
    });

    // Route for globals.js 
    server.on("/globals.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/globals.js", "text/javascript");
    });

    server.on("/main.js", HTTP_GET,[](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/main.js", "text/javascript");
    });
}

/**
 * @brief Updates the network settings by getting the parameters from the webpage
 */
void WebConfigServer::updateNetwork(){
    server.on("/updateNetwork", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("ip") && request->getParam("ip")->value() != "") {
            IPAddress ip;
            ip.fromString(request->getParam("ip")->value());
            internet.setIP(ip);
        }

        if (request->hasParam("gateway") && request->getParam("gateway")->value() != "") {
            IPAddress gateway;
            gateway.fromString(request->getParam("gateway")->value());
            internet.setGateway(gateway);
        }

        if (request->hasParam("subnet") && request->getParam("subnet")->value() != "") {
            IPAddress subnet;
            subnet.fromString(request->getParam("subnet")->value());
            internet.setSubnet(subnet);
        }

        if (request->hasParam("hostname") && request->getParam("hostname")->value() != "") {
            String hostname = request->getParam("hostname")->value();
            internet.setHostname(hostname);
        }
        request->send(200, "text/plain", "OK");
    });
}

/**
 * @brief GetNetworkSettings
 */

/**
 * @brief GetNetworkSettings
 * Creates a route "/getNetworkSettings" that returns the current network
 * settings (IP, Gateway, Subnet, Hostname) plus a boolean "dhcp" field in JSON.
 */
void WebConfigServer::GetNetworkSettings()
{
    // Register a GET endpoint that returns the network settings in JSON
    server.on("/getNetworkSettings", HTTP_GET, [this](AsyncWebServerRequest *request) {

        // Build a JSON object
        DynamicJsonDocument doc(256);

        IPAddress ip = internet.getIP();
        IPAddress gateway = internet.getGateway();
        IPAddress subnet = internet.getSubnet();
        String hostname = internet.getHostname();

        // If IP is "0.0.0.0", treat that as DHCP being enabled
        bool isDHCP = (ip == IPAddress(0, 0, 0, 0));

        // Fill the JSON object
        doc["ip"]       = ip.toString();        // e.g. "192.168.1.10"
        doc["gateway"]  = gateway.toString();   // e.g. "192.168.1.1"
        doc["subnet"]   = subnet.toString();    // e.g. "255.255.255.0"
        doc["hostname"] = hostname;             // e.g. "MyDevice"
        doc["dhcp"]     = isDHCP;               // true or false

        // Convert JSON doc to string
        String response;
        serializeJson(doc, response);

        // Send JSON back to the requester
        request->send(200, "application/json", response);
    });
}




/**
 * @brief Resets the network settings
 */
void WebConfigServer::resetNetworkSettings(){
    server.on("/resetNetworkSettings", HTTP_POST, [](AsyncWebServerRequest *request) {
        internet.resetSettings();
        request->send(200, "text/plain", "OK");
    });
}

/**
 * @brief Gets the network settings and shows them on the webpage in JSON format
 */
void WebConfigServer::getLedData(){
    server.on("/getData", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Network object
        JsonObject networkData = doc["Network"].to<JsonObject>();
        networkData["IP"] = internet.getIP().toString();
        networkData["Subnet"] = internet.getSubnet();
        networkData["Gateway"] = internet.getGateway();
        networkData["Hostname"] = internet.getHostname();

        //Serialize JSON object to string
        String response;
        serializeJsonPretty(doc, response);

        //Send response
        request->send(200, "application/json", response);
    });
    server.on("/getSegments", HTTP_GET, [](AsyncWebServerRequest *request) {
        DEBUG_PRINTF("getSegments\n");
        DynamicJsonDocument doc(2048); // Adjust size based on the expected number of segments
        JsonArray hw_led_ins = doc.createNestedArray("segments");
        // Assuming `busses.getBusCount()` returns the total number of segments
        for (int i=0; i<busses.getNumPins(); i++) {
            MyNamespace::Bus *bus = busses.getBus(i);
            if (!bus || bus->getLength()==0) break;
            JsonObject ins = hw_led_ins.createNestedObject();
            ins["start"] = bus->getStart();
            ins["len"] = bus->getLength();
            JsonArray ins_pin = ins.createNestedArray("pin");
            uint8_t pins[5];
            uint8_t nPins = bus->getPins(pins);

            DEBUG_PRINT("Stored Pins for Segment ");
            DEBUG_PRINT(i);
            DEBUG_PRINT(": ");
            for (uint8_t j = 0; j < nPins; j++) {
                DEBUG_PRINT(pins[j]);
                DEBUG_PRINT(" ");
            }
            DEBUG_PRINTLN();


            for (uint8_t i = 0; i < nPins; i++) ins_pin.add(pins[i]);
            ins["rev"] = bus->isReversed();
            ins["type"] = bus->getType() & 0x7F;
        }
        String jsonResponse;
        serializeJson(doc, jsonResponse);

        request->send(200, "application/json", jsonResponse);
    });
}

/**
 * @brief Sets controller settings
 */
void WebConfigServer::setLedData(){
   /**
    * Sets the segments of the LED strips by using a PUT request and converting the received data to a JSON object
    */
server.on("/setSegments", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
[this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Clear accumulatedData if this is the first chunk of data
    if (index == 0) {
        accumulatedData = "";
    }

    for (size_t i = 0; i < len; i++) {
        accumulatedData.concat((char)data[i]);
    }

    // Process the complete data once received
    if (index + len == total) {
        DeserializationError error = deserializeJson(doc, accumulatedData);

        if (error) {
            request->send(400, "application/json", R"({"message": "Error", "error": ")" + String(error.c_str()) + "\"}");
            accumulatedData = "";
            return;
        }

        // Extract segments array from the JSON object
        JsonArray segments = doc["segments"].as<JsonArray>();

        if (segments.isNull()) {
            request->send(400, "application/json", R"({"message": "Invalid segments array"})");
            accumulatedData = "";
            return;
        }

        for (size_t i = 0; i < MAX_PINS && i < segments.size(); i++) {
            JsonObject segment = segments[i];

            int start = segment["start"] | -1;
            int length = segment["len"] | -1;
            uint8_t gpio = segment["pin"] | -1;
            bool reversed = segment["rev"] | false;

            DEBUG_PRINT("Segment " + String(i) + ": Start=" + String(start) + ", Length=" + String(length) + ", GPIO=" + String(gpio) + ", Reversed=" + String(reversed));

            if (start < 0 || length <= 0 || gpio < 0) {
                request->send(400, "application/json", R"({"message": "Invalid parameters in segment"})");
                accumulatedData = "";
                return;
            }

            if (busConfigs[i] != nullptr) delete busConfigs[i];
            busConfigs[i] = new MyNamespace::BusConfig(
                TYPE_WS2812_RGB, &gpio, start, length, reversed, useGlobalLedBuffer);
            DEBUG_PRINTF("Config for bus %d set: Start=%d, Length=%d, GPIO=%d, Reversed=%d\n",
                         i, start, length, gpio, reversed);
        }

        request->send(200, "application/json", R"({"message": "Settings updated"})");

        bool busesChanged = true;
        doInitBusses = busesChanged;
        accumulatedData = "";
    }
});
}