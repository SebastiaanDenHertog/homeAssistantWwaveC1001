//
// Created by sebastiaan on 09/02/2026.
//

#include "c1001.h"

#include "stdio.h"

c1001::c1001(Stream *s) : _s(s) {

}

uint8_t c1001::getData(uint8_t con, uint8_t cmd, uint16_t len, uint8_t *senData, uint8_t *retData)
    {
        uint32_t timeStart = millis();
        uint32_t timeStart1 = 0;
        uint8_t data = 0, state = CMD_WHITE;
        uint16_t _len = 0;
        uint8_t count = 0;

        uint8_t cmdBuf[20];
        cmdBuf[0] = 0x53;
        cmdBuf[1] = 0x59;
        cmdBuf[2] = con;
        cmdBuf[3] = cmd;
        cmdBuf[4] = (len >> 8) & 0xff;
        cmdBuf[5] = len & 0xff;
        memcpy(&cmdBuf[6], senData, len);
        cmdBuf[6 + len] = sumData(6 + len, cmdBuf);
        cmdBuf[7 + len] = 0x54;
        cmdBuf[8 + len] = 0x43;

        while (true)
        {
            if ((millis() - timeStart1) > 1000)
            {
                while (_s->available() > 0)
                {
                    _s->read();
                }
                _s->write(cmdBuf, 9 + len);
                timeStart1 = millis();
                count = 0;
            }

            if (_s->available() > 0)
            {
                data = _s->read();
                // DBG(data);
                // timeStart1 = millis();
            }

            // Update timeout check
            if ((millis() - timeStart) > TIME_OUT)
            {
                DBG("Time out");
                return 2;
            }

            switch (state)
            {
            case CMD_WHITE:
                if (data == 0x53)
                {
                    retData[0] = data;
                    state = CMD_HEAD;
                    count = 0; // Reset count
                }
                break;
            case CMD_HEAD:
                if (data == 0x59)
                {
                    state = CMD_CONFIG;
                    retData[1] = data;
                }
                else
                {
                    state = CMD_WHITE;
                }
                break;
            case CMD_CONFIG:
                if (data == con)
                {
                    state = CMD_CMD;
                    retData[2] = data;
                }
                else
                {
                    state = CMD_WHITE;
                }
                break;
            case CMD_CMD:
                if (data == cmd)
                {
                    state = CMD_LEN_H;
                    retData[3] = data;
                }
                else
                {
                    state = CMD_WHITE;
                }
                break;
            case CMD_LEN_H:
                if(data == retData[3]){
                    state = CMD_WHITE;
                }else{
                    retData[4] = data;
                    _len = data << 8;
                    state = CMD_LEN_L;
                }
                break;
            case CMD_LEN_L:
                if(data == retData[4]){
                    state = CMD_WHITE;
                }else{
                    retData[5] = data;
                    _len |= data;
                    state = CMD_DATA;
                    DBG(_len);
                }
                break;
            case CMD_DATA:
                if (count < _len)
                {
                    retData[6 + count] = data;
                    count++;
                }
                else
                {
                    if (data == sumData(6 + count, retData))
                    {
                        retData[6 + _len] = data;
                        state = CMD_END_H;
                    }
                    else
                    {
                        state = CMD_WHITE;
                    }
                }
                break;
            case CMD_END_H:
                retData[7 + _len] = data;
                state = CMD_END_L;
                break;
            case CMD_END_L:
                retData[8 + _len] = data;
                delay(50);
                return 0;
            default:
                break;
            }

            delay(50);
        }
        delay(50);
        return 0;
    }

    uint8_t c1001::sumData(uint8_t len, uint8_t *buf)
    {
        uint16_t data = 0;
        uint8_t *_buf = buf;
        for (uint8_t i = 0; i < len; i++)
        {
            data += _buf[i];
        }
        return data & 0xff;
    }
