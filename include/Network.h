/**
 * @Authors         Tom Keuper
 * @Date created    20-10-2024
 * @Date updated    25-10-2024 (By: Tom Keuper)
 * @Description     Network class to give access to network related information
 */

#include <WiFi.h>

#ifndef NETWORK_H
#define NETWORK_H

class NetworkClass
{
public:
    IPAddress localIP();
    IPAddress subnetMask();
    IPAddress gatewayIP();
    void localMAC(uint8_t* MAC);
    bool isConnected();
    bool isEthernet();
};

extern NetworkClass Network;

#endif //NETWORK_H
