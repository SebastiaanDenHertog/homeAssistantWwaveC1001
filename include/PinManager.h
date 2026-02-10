/**
 * @Authors         Tom Keuper
 * @Date created    20-11-2024
 * @Date updated    n/a
 * @Description     Header file for helper class to initialize, control and manage IO pins
 */


#ifndef PINMANAGER_H
#define PINMANAGER_H

#include <Arduino.h>

typedef struct PinManagerPinType {
  int8_t pin;
  bool   isOutput;
} managed_pin_type;


class PinManager {
  private:
    #define NUM_PINS 50
    uint8_t pinAlloc[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 56bit, 1 bit per pin, we use 50 bits on ESP32-S3
    uint8_t ledcAlloc[2] = {0x00, 0x00}; //16 LEDC channels
  public:
    PinManager() {}
    // De-allocates a single pin
    bool deallocatePin(byte gpio);
    // Allocates a single pin, with an owner tag.
    // De-allocation requires the same owner tag (or override)
    bool allocatePin(byte gpio, bool output);

    // will return true for reserved pins
    bool isPinAllocated(byte gpio);
    // will return false for reserved pins
    bool isPinOk(byte gpio, bool output = true);

};

extern PinManager pinManager;

#endif // PINMANAGER_H
    