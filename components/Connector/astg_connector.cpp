/*
 * fiot_smart_tank.cpp
 *
 *  Created on: 24 thg 5, 2017
 *      Author: dmtan
 */

#include "atsg_connector.h"
#include "wifi_manager.h"
#include "uart_ctrl.h"
#include "config.h"
#include "mqtt_client.h"
#include "ArduinoJson.h"
#include <math.h>

const char TAG[] = "atsg_connector";
atsg_connector* atsg_connector::mInstance = NULL;
SemaphoreHandle_t atsg_connector::xSemaphore = NULL;

atsg_connector::atsg_connector()
{
    ESP_LOGD(TAG, "atsg_connector");
}

atsg_connector* atsg_connector::getInstance()
{
    if(NULL == mInstance)
    {
        mInstance = new atsg_connector();
    }
    memset(mInstance->buf, '\0', sizeof(mInstance->buf));

    return mInstance;
}


atsg_connector::~atsg_connector()
{
    ESP_LOGI(TAG, "~atsg_connector");
    mInstance = NULL;
}

void atsg_connector::setData(const uint8_t* data, const size_t len)
{
    memset(this->buf, 0x00, 1024);

    for(uint16_t i = 0; i < len; ++i)
    {
        //ESP_LOGD(TAG, "get data[%d]=%c", i, data[i]);
        this->buf[i] = data[i];
    }
    this->buf[len] = '\0';
}

bool atsg_connector::runCMD(const char* command)
{
    bool rs = false;
    if(command == NULL)
    {
        return rs;
    }
    //this->setBusy(true);
    this->createAndTakeSemaphore();
    this->setData(NULL);

    ESP_LOGI(TAG, "run command %s", command);
    uart_ctrl* mUart = uart_ctrl::getInstance();
    rs = mUart->sendData(command, true);
    if(rs == true)
    {
        uint16_t len = 0;
        uint8_t* data = mUart->readData(len);
        if(len > 0)
        {
            this->setData(data, len);
        }
    }
    
    //this->setBusy(false);
    this->releaseSemaphore();

    return rs;
}

void atsg_connector::sendData(const uint8_t* data, const size_t len)
{
    ESP_LOGD(TAG, "sendData");
    char tmp[128] = "";
    char cmd[128] = "";
    strncpy(tmp, (char*)data, len);
    sprintf(cmd, SET_CMD, tmp);
    bool rs = this->runCMD(cmd);
    if(rs == true)
    {
        //todo
    }

    //return rs;
}

void atsg_connector::getData(const uint8_t* data, const size_t len)
{
    if(len == 0)
    {
        return;
    }
    
    ESP_LOGD(TAG, "getData");
    char cmd[128] = "";
    strncpy(cmd, (char*)data, len);
    ESP_LOGI(TAG, "got command %s", cmd);
    // Get device data
    char mJsonString[200] = "";
    mJsonString[0] = '\0';

    char* sMac = wifi_sta::getInstance()->getMacAddress();
    uint64_t ts = wifi_sta::getInstance()->getTimeStamp();

    sprintf(mJsonString, "{"
            "\"gateway_mac_address\":\"%s\","
            "\"gateway_name\":%d,"
            "\"gateway_type\":%d,"
            "\"timestamp\":%s,"
            "\"freemem\":%d,"
            "\"device_rx_cmd\":\"%s\""
            "}",
        sMac,
        DB_DEVICE_ATSG_CONTROLLER,
        DB_DEVICE_TYPE_CONTROLLER,
        uint642Char(ts),
        esp_get_free_heap_size(),
        data);
    ESP_LOGI(TAG, "json cmd %s", mJsonString);
    mqtt_client *mqtt = mqtt_client::getInstance();
    mqtt->sendMessage(mJsonString);
    //return mJsonString;
    //return rs;
}

void atsg_connector::handleData(const char* json)
{
    ESP_LOGD(TAG, "getData: %s", json);
    DynamicJsonBuffer jsonBuffer(1024);
    JsonObject &device = jsonBuffer.parseObject(json);
    const char* mac = device["device_mac_address"];
    const char* cmd = device["device_tx_cmd"];

    ESP_LOGD(TAG, "cmd: %s", cmd);
    if(strcmp(cmd, "ping") == 0)
    {
        char response[] = "ok";
        this->getData((uint8_t*)response, strlen(response));
        return;
    }
    this->sendData((uint8_t*)cmd, strlen(cmd));
}

void atsg_connector::createAndTakeSemaphore()
{
    if(NULL == xSemaphore)
    {
        xSemaphore = xSemaphoreCreateMutex();
    }

    while( xSemaphoreTake( xSemaphore, ( TickType_t ) 100 ) == 0 );
}

void atsg_connector::releaseSemaphore()
{
    xSemaphoreGive( xSemaphore );
}
