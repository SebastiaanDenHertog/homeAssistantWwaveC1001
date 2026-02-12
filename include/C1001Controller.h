/**
 * @Authors         Tom Keuper
 * @Date created    20-10-2024
 * @Date updated    25-10-2024 (By: Tom Keuper)
 * @Description     Main controller for the C1001 MMWave controller system
 */

#ifndef C1000CONTROLLER_H
#define C1000CONTROLLER_H



// GLOBAL VARIABLES
// both declared and defined in header (solution from http://www.keil.com/support/docs/1868.htm)
//
//e.g. byte test = 2 becomes CONTROLLER_GLOBAL byte test _INIT(2);
//     int arr[]{0,1,2} becomes CONTROLLER_GLOBAL int arr[] _INIT_N(({0,1,2}));

#ifndef C1001_CONTROLLER_DEFINE_GLOBAL_VARS
# define CONTROLLER_GLOBAL extern
# define _INIT(x)
# define _INIT_N(x)
#else
# define CONTROLLER_GLOBAL
# define _INIT(x) = x

// Needed to ignore commas in array definitions
#define UNPACK( ... ) __VA_ARGS__
# define _INIT_N(x) UNPACK x
#endif

#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)


// Libraries
#include <DFRobot_HumanDetection.h>
#include <WiFi.h>
#include <ETH.h>
#include <Preferences.h>
#include "const.h"
#include "Network.h"
#include "internet.h"
#include "WebConfigServer.h"
#include <LittleFS.h>

// Global Variable definitions

// global ArduinoJson buffer
#define JSON_BUFFER_SIZE 24576

CONTROLLER_GLOBAL StaticJsonDocument<JSON_BUFFER_SIZE> doc;
CONTROLLER_GLOBAL volatile uint8_t jsonBufferLock _INIT(0);
CONTROLLER_GLOBAL JsonDocument* fileDoc;
CONTROLLER_GLOBAL bool doCloseFile _INIT(false);
CONTROLLER_GLOBAL size_t fsBytesUsed _INIT(0);
CONTROLLER_GLOBAL size_t fsBytesTotal _INIT(0);
CONTROLLER_GLOBAL bool doSerializeConfig _INIT(false);        // flag to initiate saving of config

CONTROLLER_GLOBAL IPAddress realtimeIP _INIT_N(((0, 0, 0, 0)));

// Network CONFIG
CONTROLLER_GLOBAL IPAddress staticIP      _INIT_N(((  0,   0,  0,  0))); // static IP of ESP

class C1001Controller {
public:
    C1001Controller();
    static C1001Controller& instance()
    {
        static C1001Controller instance;
        return instance;
    }

    // boot starts here
    void setup();
    void loop();

};

#endif //C1001CONYTOLER_H
