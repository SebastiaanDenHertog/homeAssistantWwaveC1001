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
    static unsigned long lastReceivedTime = millis();
    static bool relayOn = false;
    static unsigned long relayOffTime = 0; // Time when the relay was turned off
    const unsigned long relayOnDelay = 20000; // 3 seconds delay before turning the relay back on

    if (doInitBusses)
    {
        doInitBusses = false;
        initBusses();
        doSerializeConfig = true; // Save Config
        Internet::autoRegisterWithOrchestrator();
    }
    if (doSerializeConfig) serializeConfig();

    if (e131NewData && millis() - strip.getLastShow() > 15)
    {
        e131NewData = false;
        DEBUG_PRINTF("TTS: %d\n", millis() - strip.getLastShow());
        strip.show();

        // Update the last received time and ensure relay is turned ON
        lastReceivedTime = millis();
        // Only turn on the relay if the delay has passed since it was turned off
        if (!relayOn && (millis() - relayOffTime > relayOnDelay)) {
            digitalWrite(33, HIGH); // Turn on relay
            relayOn = true;
            DEBUG_PRINTLN("Relay turned ON");
        }
    }

    // Check if no data has been received for 60 seconds
    if (millis() - lastReceivedTime > 60000) {
        if (relayOn) {
            digitalWrite(33, LOW); // Turn off relay
            relayOn = false;
            relayOffTime = millis(); // Record the time when the relay is turned off
            DEBUG_PRINTLN("Relay turned OFF - Timeout");
        }
    }
}

void C1001Controller::initControllerSettings()
{

    bool fsinit = false;
    DEBUGFS_PRINTLN(F("Mounting FS"));
    fsinit = LittleFS.begin(true);
    if (!fsinit) {
        DEBUGFS_PRINTLN(F("FS failed!"));
        return;
    }
    updateFSInfo();

    DEBUG_PRINTLN(F("Reading config"));
    deserializeConfigFromFS();

}

void C1001Controller::beginStrip()
{
    // Initialize NeoPixel Strip and button
    strip.finalizeInit(); // busses created during deserializeConfig()
    strip.makeAutoSegments();
    strip.setBrightness(255);

    if (true) {
        if (briS > 0) bri = briS;
        else if (bri == 0) bri = 128;
    }

    strip.fill(0x000000); // Always reset to BLACK
    strip.show();

    // TODO Add Relay function to turn on Power Supply
}



void C1001Controller::initBusses()
{
    DEBUG_PRINTLN(F("Re-init busses."));
    bool aligned = strip.checkSegmentAlignment(); //see if old segments match old bus(ses)
    busses.removeAll();
    uint32_t mem = 0, globalBufMem = 0;
    uint16_t maxlen = 0;
    for (uint8_t i = 0; i < MAX_PINS; i++) {
        if (busConfigs[i] == nullptr) break;
        mem += MyNamespace::BusManager::memUsage(*busConfigs[i]);
        if (useGlobalLedBuffer && busConfigs[i]->start + busConfigs[i]->count > maxlen) {
            maxlen = busConfigs[i]->start + busConfigs[i]->count;
            globalBufMem = maxlen * 4;
        }
        if (mem + globalBufMem <= MAX_LED_MEMORY) {
            busses.add(*busConfigs[i]);
        }
        delete busConfigs[i]; busConfigs[i] = nullptr;
    }
    strip.finalizeInit(); // also loads default ledmap if present
    if (aligned) strip.makeAutoSegments();
    else strip.fixInvalidSegments();