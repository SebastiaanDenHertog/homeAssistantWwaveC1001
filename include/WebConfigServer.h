/**
 * @Authors:            Laurens Leusink, Tom Keuper
 * @Date created:       16-09-2024
 * @Date updated:       23-11-2024 (By: Tom Keuper)
 * @Description:        Header file for the WebServer.cpp file.
 */

#include <Wifi.h>
#include <ESPAsyncWebServer.h>
#include "C1001Controller.h"
#include <ArduinoJson.h>

class WebConfigServer {
public:
    WebConfigServer();
    static String processor(const String& var);
    void begin();
    void getLedData();
    void setLedData();
    void GetNetworkSettings();
private:
    AsyncWebServer server;
    const char* password;
    
    void setupRoutes();
    void updateNetwork();
    void resetNetworkSettings();
    JsonDocument doc;
    String accumulatedData; // For adding up the data chunks
};