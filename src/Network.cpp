/**
 * @Authors         Tom Keuper
 * @Date created    20-10-2024
 * @Date updated    25-10-2024 (By: Tom Keuper)
 * @Description     Class to retreive network related information
 */

#include "Network.h"

IPAddress NetworkClass::localIP()
{
    IPAddress localIP;
    localIP = ETH.localIP();
    if (localIP[0] != 0) {
        return localIP;
    }

    return INADDR_NONE;
}

IPAddress NetworkClass::subnetMask()
{
    if (ETH.localIP()[0] != 0) {
        return ETH.subnetMask();
    }
    return IPAddress(255, 255, 255, 0);
}

IPAddress NetworkClass::gatewayIP()
{
    if (ETH.localIP()[0] != 0) {
        return ETH.gatewayIP();
    }
    return INADDR_NONE;
}

void NetworkClass::localMAC(uint8_t* MAC)
{
    // Start work around
    String macString = ETH.macAddress();
    char macChar[18];
    char * octetEnd = macChar;

    strlcpy(macChar, macString.c_str(), 18);

    for (uint8_t i = 0; i < 6; i++) {
        MAC[i] = (uint8_t)strtol(octetEnd, &octetEnd, 16);
        octetEnd++;
    }
    // End work around

    for (uint8_t i = 0; i < 6; i++) {
        if (MAC[i] != 0x00) {
            return;
        }
    }
}

bool NetworkClass::isConnected()
{
    return (WiFi.localIP()[0] != 0 && WiFi.status() == WL_CONNECTED) || ETH.localIP()[0] != 0;
}

bool NetworkClass::isEthernet()
{
    return (ETH.localIP()[0] != 0);
}

NetworkClass Network;