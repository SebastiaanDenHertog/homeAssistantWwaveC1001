/**
 * @Authors         Tom Keuper
 * @Date created    23-10-2024
 * @Date updated    25-10-2024 (By: Tom Keuper)
 * @Description     File system controller for reading and writing to the LittleFS file system
 */

#include "C1001Controller.h"

//simple macro for ArduinoJSON's or syntax
#define CJSON(a,b) a = b | a

//threading/network callback details: https://github.com/Aircoookie/WLED/pull/2336#discussion_r762276994
bool requestJSONBufferLock(uint8_t module)
{
  unsigned long now = millis();

  while (jsonBufferLock && millis()-now < 1000) delay(1); // wait for a second for buffer lock

  if (millis()-now >= 1000) {
    DEBUG_PRINT(F("ERROR: Locking JSON buffer failed! ("));
    DEBUG_PRINT(jsonBufferLock);
    DEBUG_PRINTLN(")");
    return false; // waiting time-outed
  }

  jsonBufferLock = module ? module : 255;
  DEBUG_PRINT(F("JSON buffer locked. ("));
  DEBUG_PRINT(jsonBufferLock);
  DEBUG_PRINTLN(")");
  fileDoc = &doc;  // used for applying presets (presets.cpp)
  doc.clear();
  return true;
}


void releaseJSONBufferLock()
{
  DEBUG_PRINT(F("JSON buffer released. ("));
  DEBUG_PRINT(jsonBufferLock);
  DEBUG_PRINTLN(")");
  fileDoc = nullptr;
  jsonBufferLock = 0;
}


void serializeConfig() {
  DEBUG_PRINTLN(F("Writing settings to /cfg.json..."));

  if (!requestJSONBufferLock(2)) return;

  JsonArray hw_led_ins = doc.createNestedArray("segments");

  for (uint8_t s = 0; s < busses.getNumPins(); s++) {
    MyNamespace::Bus *bus = busses.getBus(s);
    if (!bus || bus->getLength() == 0) break;
    JsonObject ins = hw_led_ins.createNestedObject();
    ins["start"] = bus->getStart();
    ins["len"] = bus->getLength();
    JsonArray ins_pin = ins.createNestedArray("pin");
    uint8_t pins[5];
    uint8_t nPins = bus->getPins(pins);
    for (uint8_t i = 0; i < nPins; i++) ins_pin.add(pins[i]);
    ins["rev"] = bus->isReversed();
    ins[F("skip")] = bus->skippedLeds();
    ins["type"] = bus->getType() & 0x7F;
  }

  JsonObject def = doc.createNestedObject("def");
  def["bri"] = briS;

  JsonObject dmx = doc.createNestedObject("dmx");
  dmx[F("chan")] = DMXChannels;
  dmx[F("gap")] = DMXGap;
  dmx["start"] = DMXStart;
  dmx[F("start-led")] = DMXStartLED;

  dmx[F("e131proxy")] = e131ProxyUniverse;

  // Write the JSON to the file
  File f = LittleFS.open("/cfg.json", "w");
  if (f) {
    serializeJson(doc, f);
    f.close();
  } else {
    DEBUG_PRINTLN(F("Error: Unable to open /cfg.json for writing!"));
  }

  releaseJSONBufferLock();

  doSerializeConfig = false;
}

bool deserializeConfig(JsonObject doc, bool fromFS) {
  DEBUGFS_PRINTLN("Deserializing config");
  bool needsSave = false;

  // initialize LED pins and lengths prior to other HW (except for ethernet)

  JsonArray ins = doc["segments"];

  if (fromFS || !ins.isNull()) {
    DEBUGFS_PRINTLN("Deserializing segments");
    uint8_t s = 0;  // bus iterator
    if (fromFS) busses.removeAll(); // can't safely manipulate busses directly in network callback
    uint32_t mem = 0, globalBufMem = 0;
    uint16_t maxlen = 0;
    bool busesChanged = false;

    // Process each segment in the "segments" array
    for (JsonObject elm : ins) {
      DEBUGFS_PRINTF("Processing segment %d\n", s + 1);

      if (s >= MAX_PINS) {
        DEBUGFS_PRINTF("Reached the maximum number of pins (%d). Stopping...\n", MAX_PINS);
        break;
      }

      uint8_t pins[5] = {255, 255, 255, 255, 255};
      JsonArray pinArr = elm["pin"];
      if (pinArr.size() == 0) {
        DEBUGFS_PRINTLN(F("No pins specified in the segment. Skipping..."));
        continue;
      }

      pins[0] = pinArr[0];
      uint8_t i = 0;
      for (int p : pinArr) {
        pins[i++] = p;
        if (i > 4) break;
      }

      uint16_t length = elm["len"] | 1;
      uint16_t start = elm["start"] | 0;
      if (length == 0 || start + length > MAX_LEDS) {
        DEBUGFS_PRINTF("Invalid segment (len: %d, start: %d). Skipping...\n", length, start);
        continue;
      }

      uint8_t ledType = elm["type"] | TYPE_WS2812_RGB;
      bool reversed = elm["rev"];
      bool refresh = elm["ref"] | false;
      ledType |= refresh << 7; // hack bit 7 to indicate strip requires off refresh

      DEBUGFS_PRINTF("Segment info: len=%d, start=%d, type=%d, rev=%d, refresh=%d\n",
                     length, start, ledType, reversed, refresh);
      DEBUGFS_PRINT("Pin configuration: ");
      for (uint8_t j = 0; j < i; j++) {
        DEBUGFS_PRINTF("%d ", pins[j]);
      }
      DEBUGFS_PRINTLN();

      if (fromFS) {
        // Configure bus from file system
        MyNamespace::BusConfig bc = MyNamespace::BusConfig(ledType, pins, start, length, reversed, useGlobalLedBuffer);
        mem += MyNamespace::BusManager::memUsage(bc);
        if (useGlobalLedBuffer && start + length > maxlen) {
          maxlen = start + length;
          globalBufMem = maxlen * 4;
        }
        if (mem + globalBufMem <= MAX_LED_MEMORY) {
          if (busses.add(bc) == -1) {
            DEBUGFS_PRINTLN(F("Failed to add bus. Stopping..."));
            break;
          }
        }
      } else {
        // Update bus configuration
        if (busConfigs[s] != nullptr) delete busConfigs[s];
        busConfigs[s] = new MyNamespace::BusConfig(ledType, pins, start, length, reversed, useGlobalLedBuffer);
        busesChanged = true;
      }
      s++;
    }
    doInitBusses = busesChanged;
    // finalization done in beginStrip()
    DEBUGFS_PRINTLN(F("Finished processing segments."));
  }

  // Deserialize DMX configuration
  JsonObject dmx = doc["dmx"];
  if (!dmx.isNull()) {
    DEBUGFS_PRINTLN(F("Deserializing DMX configuration..."));
  }
  CJSON(DMXChannels, dmx[F("chan")]);
  CJSON(DMXGap, dmx[F("gap")]);
  CJSON(DMXStart, dmx["start"]);
  CJSON(DMXStartLED, dmx[F("start-led")]);

  JsonArray dmx_fixmap = dmx[F("fixmap")];
  for (int i = 0; i < dmx_fixmap.size(); i++) {
    if (i > 14) break;
    CJSON(DMXFixtureMap[i], dmx_fixmap[i]);
  }

  CJSON(e131ProxyUniverse, dmx[F("e131proxy")]);

  if (fromFS) return needsSave;

  // If the deserialization was from /json/cfg
  if (doInitBusses) return false; // No save needed, will occur after bus init in wled.cpp loop

  return (doc["sv"] | true);
}

void deserializeConfigFromFS() {
  if (!requestJSONBufferLock(1)) return;

  DEBUG_PRINTLN(F("Reading settings from /cfg.json..."));

  // Attempt to read the JSON object from the file
  bool success = readObjectFromFile("/cfg.json", nullptr, &doc);
  if (!success) {
    DEBUG_PRINTLN(F("Error: Failed to read configuration file; attempting to write default config..."));

    releaseJSONBufferLock();

    serializeConfig();
    // Initialize Ethernet or perform other fallback procedures if necessary
    // TODO: Check if we want to store Ethernet settings and initialize Ethernet here.
    return;
  }

  // Debug print the contents of the doc variable
  DEBUG_PRINTLN(F("Deserialized JSON from /cfg.json:"));
  String jsonStr;
  serializeJson(doc, jsonStr); // Serialize the JSON object to a string
  DEBUG_PRINTLN(jsonStr);      // Log the string representation of the JSON content

  DEBUG_PRINTLN(F("Deserializing configuration from JSON object..."));
  deserializeConfig(doc.as<JsonObject>(), true);

  releaseJSONBufferLock();
}
