/**
 * @Authors:            Laurens Leusink, Sebastiaan den Hertog, Tom Keuper
 * @Date created:       10-10-2024
 * @Date updated:       26-11-2024 (By: Tom Keuper)
 * @Description:        Header file for handling the internet, TCP, and UDP connections of the ESP32.
 */

#ifndef INTERNET_H
#define INTERNET_H

#include <ETH.h>
#include <WiFi.h>
#include <Preferences.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <functional>
#include <HTTPClient.h>

class Internet {
public:
    static bool eth_connected;

    Internet();
    void begin();
    void testClient(const char *host, uint16_t port); 
    void getSettings();
    void updateSettings();
    void applySettings();    
    void displaySettings();

    void setIP(IPAddress ip);
    void setGateway(IPAddress gateway);
    void setSubnet(IPAddress subnet);
    void setHostname(String hostname);
    void resetSettings();

    IPAddress getIP();
    IPAddress getGateway();
    IPAddress getSubnet();
    const char* getHostname();
    String getMAC();
    bool getLinkStatus();
    uint8_t getLinkSpeed();
    String getControllerName();

    static void autoRegisterWithOrchestrator();

private:
    WiFiUDP udp;
    WiFiServer *tcpServer;
    WiFiClient tcpClient;
    
    bool useTcp;
    uint16_t port;

    uint8_t phy_addr = 0;
    int power = 5;
    int mdc = 23;
    int mdio = 18;
    eth_phy_type_t type = ETH_PHY_LAN8720;
    eth_clock_mode_t clock_mode = ETH_CLOCK_GPIO17_OUT;

    struct NetworkSettings {
        IPAddress ip = IPAddress(0, 0, 0, 0);
        IPAddress gateway = IPAddress(0, 0, 0, 0);
        IPAddress subnet = IPAddress(0, 0, 0, 0);
        String hostname = "";
    } networkSettings;
    
    std::function<void(uint16_t universe, uint16_t length, uint8_t* data)> dmxCallback;

    static void WiFiEvent(WiFiEvent_t event);

    static String createRegistrationPayload();
    static void updateControllerData();

    static String getSegmentsJson(int &totalLedCount);

};

#endif
