/*
 ESP8266WiFiScan.h - esp8266 Wifi support.
 Based on WiFi.h from Ardiono WiFi shield library.
 Copyright (c) 2011-2014 Arduino.  All right reserved.
 Modified by Ivan Grokhotkov, December 2014
 Reworked by Markus Sattler, December 2015

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ESP32WIFISCAN_H_
#define ESP32WIFISCAN_H_

extern "C" {
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
//#include <esp32-hal.h>
#include <lwip/ip_addr.h>
#include "lwip/err.h"
}

class WiFiScanClass
{

public:

    int8_t scanNetworks(bool async = false, bool show_hidden = true);

    int8_t scanComplete();
    void scanDelete();

    // scan result
    bool getNetworkInfo(uint8_t networkItem, char* ssid, uint8_t &encryptionType, int32_t &RSSI, uint8_t* &BSSID, int32_t &channel);

    char* SSID(uint8_t networkItem);
    wifi_auth_mode_t encryptionType(uint8_t networkItem);
    int32_t RSSI(uint8_t networkItem);
    uint8_t * BSSID(uint8_t networkItem);
    char* BSSIDstr(uint8_t networkItem);
    int32_t channel(uint8_t networkItem);

    static void _scanDone();
protected:

    static bool _scanAsync;
    static bool _scanStarted;
    static bool _scanComplete;

    static uint16_t _scanCount;
    static void* _scanResult;

    static void * _getScanInfoByIndex(int i);

};


#endif /* ESP32WIFISCAN_H_ */
