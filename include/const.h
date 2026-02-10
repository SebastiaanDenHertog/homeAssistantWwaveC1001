/**
 * @Authors         Tom Keuper
 * @Date created    19-10-2024
 * @Date updated    26-10-2024 (By: Tom Keuper)
 * @Description     Constants file for the project
 */

#ifndef CONST_H
#define CONST_H
#include <Arduino.h>

//#define ESP_DEBUG
//#define ESP_DEBUG_FS

#define DEFAULT_LED_COUNT 30
#ifndef MAX_LEDS_PER_BUS
    #define MAX_LEDS_PER_BUS 2048   // may not be enough for fast LEDs (i.e. APA102)
#endif

#ifndef MAX_LEDS
    #define MAX_LEDS 8192
#endif

#ifndef MAX_LED_MEMORY
    #define MAX_LED_MEMORY 64000
#endif


#define MAX_PINS 10

#define TYPE_NONE                 0            //light is not configured
#define TYPE_RESERVED             1            //unused. Might indicate a "virtual" light
//Digital types (data pin only) (16-31)
#define TYPE_WS2812_1CH          18            //white-only chips (1 channel per IC) (unused)
#define TYPE_WS2812_1CH_X3       19            //white-only chips (3 channels per IC)
#define TYPE_WS2812_2CH_X3       20            //CCT chips (1st IC controls WW + CW of 1st zone and CW of 2nd zone, 2nd IC controls WW of 2nd zone and WW + CW of 3rd zone)
#define TYPE_WS2812_WWA          21            //amber + warm + cold white
#define TYPE_WS2812_RGB          22
#define TYPE_GS8608              23            //same driver as WS2812, but will require signal 2x per second (else displays test pattern)
#define TYPE_WS2811_400KHZ       24            //half-speed WS2812 protocol, used by very old WS2811 units
#define TYPE_TM1829              25
#define TYPE_UCS8903             26
#define TYPE_UCS8904             29
#define TYPE_SK6812_RGBW         30
#define TYPE_TM1814              31
//"Analog" types (PWM) (32-47)
#define TYPE_ONOFF               40            //binary output (relays etc.)
#define TYPE_ANALOG_1CH          41            //single channel PWM. Uses value of brightest RGBW channel
#define TYPE_ANALOG_2CH          42            //analog WW + CW
#define TYPE_ANALOG_3CH          43            //analog RGB
#define TYPE_ANALOG_4CH          44            //analog RGBW
#define TYPE_ANALOG_5CH          45            //analog RGB + WW + CW
//Digital types (data + clock / SPI) (48-63)
#define TYPE_WS2801              50
#define TYPE_APA102              51
#define TYPE_LPD8806             52
#define TYPE_P9813               53
#define TYPE_LPD6803             54
//Network types (master broadcast) (80-95)
#define TYPE_NET_DDP_RGB         80            //network DDP RGB bus (master broadcast bus)
#define TYPE_NET_E131_RGB        81            //network E131 RGB bus (master broadcast bus, unused)
#define TYPE_NET_ARTNET_RGB      82            //network ArtNet RGB bus (master broadcast bus, unused)
#define TYPE_NET_DDP_RGBW        88            //network DDP RGBW bus (master broadcast bus)


#define REALTIME_MODE_E131        4
#define REALTIME_MODE_ADALIGHT    5
#define REALTIME_MODE_ARTNET      6
#define REALTIME_MODE_TPM2NET     7
#define REALTIME_MODE_DDP         8

// DMX Modes
#define DMX_MODE_DISABLED         0            //not used
#define DMX_MODE_MULTIPLE_RGB     4            //every LED is addressed with its own RGB (ledCount * 3 channels)
#define DMX_MODE_MULTIPLE_DRGB    5            //every LED is addressed with its own RGB and share a master dimmer (ledCount * 3 + 1 channels)
#define DMX_MODE_MULTIPLE_RGBW    6            //every LED is addressed with its own RGBW (ledCount * 4 channels)

#define E131_MAX_UNIVERSE_COUNT 50

#define DEBUGOUT Serial

#ifdef ESP_DEBUG
    #define DEBUG_PRINT(x) DEBUGOUT.print(x)
    #define DEBUG_PRINTLN(x) DEBUGOUT.println(x)
    #define DEBUG_PRINTF(x...) DEBUGOUT.printf(x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(x...)
#endif

#ifdef ESP_DEBUG_FS
  #define DEBUGFS_PRINT(x) DEBUGOUT.print(x)
  #define DEBUGFS_PRINTLN(x) DEBUGOUT.println(x)
  #define DEBUGFS_PRINTF(x...) DEBUGOUT.printf(x)
#else
  #define DEBUGFS_PRINT(x)
  #define DEBUGFS_PRINTLN(x)
  #define DEBUGFS_PRINTF(x...)
#endif


#define IS_DIGITAL(t) ((t) & 0x10) //digital are 16-31 and 48-63
#define IS_PWM(t)     ((t) > 40 && (t) < 46)
#define NUM_PWM_PINS(t) ((t) - 40) //for analog PWM 41-45 only
#define IS_2PIN(t)      ((t) > 47)

#endif //CONST_H
