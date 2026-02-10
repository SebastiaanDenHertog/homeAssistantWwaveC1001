//
// Created by sebastiaan on 09/02/2026.
//

#ifndef C1001_H
#define C1001_H

#include "Arduino.h"
#if (defined ARDUINO_AVR_UNO) && (defined ESP8266)
#include "SoftwareSerial.h"
#else
#include "HardwareSerial.h"
#endif

class c1001 {
    public:
    c1001(Stream *s);
    ~c1001();
    uint8_t getData(uint8_t con, uint8_t cmd, uint16_t len, uint8_t *senData, uint8_t *retData);
    uint8_t sumData(uint8_t len, uint8_t *buf);
    Stream *_s = NULL;
    private:

};

#endif //C1001_H