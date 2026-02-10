#include <Arduino.h>
#include <c1001.h>

c1001 c1(&Serial2);

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 16, 17);

    uint8_t data= 0x0f;
    uint8_t buff[10];
    while (c1.getData(0x01,0x83,1,&data,buff)==0) {
        Serial.print(".");
        delay(100);
    }
}