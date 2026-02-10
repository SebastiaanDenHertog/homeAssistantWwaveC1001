/**
 * @Authors         Tom Keuper
 * @Date created    20-10-2024
 * @Date updated    23-10-2024 (By: Tom Keuper)
 * @Description     Manager class to initialize and control pin out puts
 */

#include "PinManager.h"

bool PinManager::allocatePin(byte gpio, bool output)
{
  // HW I2C & SPI pins have to be allocated using allocateMultiplePins variant since there is always SCL/SDA pair
  if (!isPinOk(gpio, output) || (gpio >= NUM_PINS)) {
#ifdef ESP_DEBUG
    if (gpio < 255) {  // 255 (-1) is the "not defined GPIO"
      if (!isPinOk(gpio, output)) {
        DEBUG_PRINT(F("PIN ALLOC: FAIL for owner "));
        DEBUG_PRINT(F(": GPIO ")); DEBUG_PRINT(gpio);
        if (output) DEBUG_PRINTLN(F(" cannot be used for i/o on this MCU."));
        else DEBUG_PRINTLN(F(" cannot be used as input on this MCU."));
      } else {
        DEBUG_PRINT(F("PIN ALLOC: FAIL: GPIO ")); DEBUG_PRINT(gpio);
        DEBUG_PRINTLN(F(" - HW I2C & SPI pins have to be allocated using allocateMultiplePins()"));
      }
    }
#endif
    return false;
  }
  if (isPinAllocated(gpio)) {
#ifdef ESP_DEBUG
    DEBUG_PRINT(F("PIN ALLOC: Pin "));
    DEBUG_PRINT(gpio);
    DEBUG_PRINT(F(" already allocated by "));
    DEBUG_PRINTLN(F(""));
#endif
    return false;
  }

  byte by = gpio >> 3;
  byte bi = gpio - 8*by;
  bitWrite(pinAlloc[by], bi, true);
#ifdef WLED_DEBUG
  DEBUG_PRINT(F("PIN ALLOC: Pin "));
  DEBUG_PRINT(gpio);
  DEBUG_PRINT(F(" successfully allocated by "));
  DebugPrintOwnerTag(tag);
  DEBUG_PRINTLN(F(""));
#endif

  return true;
}

/// Actual allocation/deallocation routines
bool PinManager::deallocatePin(byte gpio)
{
    if (gpio == 0xFF) return true;           // explicitly allow clients to free -1 as a no-op
    if (!isPinOk(gpio, false)) return false; // but return false for any other invalid pin

    byte by = gpio >> 3;
    byte bi = gpio - 8*by;
    bitWrite(pinAlloc[by], bi, false);
    return true;
}


// Check if supplied GPIO is ok to use
// https://quinled.info/quinled-dig-octa-brainboard-32-8l-pinout-guide/
bool PinManager::isPinOk(byte gpio, bool output)
{
  if (digitalPinIsValid(gpio)) {
    // strapping pins: 0, 45 & 46
    if (gpio > 21 && gpio < 33) return false;     // 22 to 32: not connected + SPI FLASH
    // JTAG: GPIO39-42 are usually used for inline debugging
    // GPIO46 is input only and pulled down

    if (output) return digitalPinCanOutput(gpio);
    return true;
  }

  return false;
}

// if tag is set to PinOwner::None, checks for ANY owner of the pin.
// if tag is set to any other value, checks if that tag is the current owner of the pin.
bool PinManager::isPinAllocated(byte gpio)
{
  if (!isPinOk(gpio, false)) return true;
  if (gpio >= NUM_PINS) return false; // catch error case, to avoid array out-of-bounds access
  byte by = gpio >> 3;
  byte bi = gpio - (by<<3);
  return bitRead(pinAlloc[by], bi);
}

PinManager pinManager = PinManager();