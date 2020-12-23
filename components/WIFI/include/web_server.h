/*
 * web_server.h
 *
 *  Created on: 27 thg 3, 2017
 *      Author: dmtan
 */

#ifndef COMPONENTS_WIFI_INCLUDE_WEB_SERVER_H_
#define COMPONENTS_WIFI_INCLUDE_WEB_SERVER_H_
#include "mongoose.h"
#include "sdkconfig.h"
extern "C" {
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <nvs_flash.h>
#include <nvs.h>
}

class web_server
{
private:
    web_server();
    virtual ~web_server();

public:
    uint64_t mLastUpdate;
    void start();
    static web_server* getInstance();

private:
    static void handleOTAPage(struct mg_connection *nc, http_message *hm);
    static void handleCheckNewFirmware(struct mg_connection *nc);
    static void handleRebootDevice(struct mg_connection *nc, http_message *hm);
    static void handleConnectWifi(struct mg_connection *nc, http_message *hm);
    static void handleContents(struct mg_connection *nc, http_message *hm, const char* uri);
    static void handleSetting(struct mg_connection *nc, http_message *hm);
    static void handleNotFound(struct mg_connection *nc, http_message *hm);
    static void handleScanWiFi(struct mg_connection *nc);
    static void handleGetData(struct mg_connection *nc);
    static void handleSetProductSerial(struct mg_connection *nc, http_message *hm);
    static void handleGetDeviceStats(struct mg_connection *nc);
    //static void handleGetSwitch(struct mg_connection *nc, http_message *hm);
    static void handleSetSwitch(struct mg_connection *nc, http_message *hm);
    static void handleSetIdle(struct mg_connection *nc, http_message *hm);
    static void handleSetMode(struct mg_connection *nc, http_message *hm);
    static void handleFactoryReset(struct mg_connection *nc);
    static void startTask(void *data);
    static void event_handler(struct mg_connection *nc, int ev, void *evData);

    static void handleRunota_updateFirmware(struct mg_connection *nc, http_message *hm);
    static void handleGetSystemStatistic(struct mg_connection *nc);
    static void handle_ota_update(struct mg_connection *nc, int ev, void *ev_data);

private:
    static web_server *s_pInstance;
    static const char s_Tag[];
};

#endif /* COMPONENTS_WIFI_INCLUDE_WEB_SERVER_H_ */
