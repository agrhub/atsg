/*
  * WiFiConnectionManager
  * This class is use to manage connect from gateway to Xiaomi Flora device
  *
  * Created: March 11, 2017
  * Author: Tan Do
  * Company: AgrHub
  * Email: dmtan90@gmail.com
  * Website: agrhub.com
*/

#ifndef CONNECTION_h
#define CONNECTION_h

#include "config.h"
#include "setting.h"
#include "wifi_ap.h"
#include "wifi_sta.h"
#include "web_server.h"
#include "socket_ota_update.h"
#include <tcpip_adapter.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_task_wdt.h>
#include <lwip/ip_addr.h>
#include <esp_wifi.h>
#include <freertos/task.h>
#include <freertos/list.h>

class wifi_manager
{
public:
    void init();
    void startService();
    void connectSTA();
    void createAP();

    static wifi_manager* getInstance();
    static char* mac_to_ip_address(const char* mac);
private:
    static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);
private:
    wifi_manager();
    ~wifi_manager();

private:
    static wifi_manager* s_pInstance;
    static uint16_t s_staTimeout;
    static const char s_Tag[];
    static SemaphoreHandle_t s_Semaphore;
    static SemaphoreHandle_t s_SemaphoreMac2Ip;

};

#endif
