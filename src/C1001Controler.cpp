#define CONTROLLER_DEFINE_GLOBAL_VARS
#include "C1001Controller.h"

Internet internet;
WebConfigServer webServer;

C1001Controller::C1001Controller() = default;


void C1001Controller::setup() {
    Serial.begin(115200);
    delay(2500);
    DEBUG_PRINTLN("Starting Controller Controller");

    DEBUG_PRINTLN("Starting internet");
    internet.begin();
    webServer.begin();

    initControllerSettings();

    DEBUG_PRINTLN(F("Initializing strip"));
    beginStrip();
    DEBUG_PRINT(F("heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());

    delay(1000);

    if (e131.begin(e131Multicast, e131Port, e131Universe, E131_MAX_UNIVERSE_COUNT)) {
        DEBUG_PRINTLN("E1.31 setup complete.");
    } else {
        DEBUG_PRINTLN("E1.31 setup failed.");
    }

    delay(300);
    DEBUG_PRINTLN("ArtNet setup complete.");

    Internet::autoRegisterWithOrchestrator();
}

void C1001Controller::loop() {

}