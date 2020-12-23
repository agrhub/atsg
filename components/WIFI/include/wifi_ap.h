/*
  * wifi_ap
  * This class is use to manage connect from gateway to Xiaomi Flora device
  *
  * Created: March 11, 2017
  * Author: Tan Do
  * Company: AgrHub
  * Email: dmtan90@gmail.com
  * Website: agrhub.com
*/

#ifndef wifi_ap_h
#define wifi_ap_h

#include "mongoose.h"

extern "C" {
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
}

class wifi_ap
{
private:
    static wifi_ap* s_pInstance;
public:
    static wifi_ap* getInstance();

private:
    wifi_ap();
    ~wifi_ap();

    static const char s_Tag[];
public:
    void init(char* ssid, char* pwd);
    bool isCreated();
};

#endif
