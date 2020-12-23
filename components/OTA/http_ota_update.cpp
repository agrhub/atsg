#include "http_ota_update.h"
#include "ArduinoJson.h"
#include "ota_update.h"
#include "config.h"
#include "wifi_sta.h"
#include "http_request.h"
#include "sys_ctrl.h"
#include "led_ctrl.h"
#include "mqtt_client.h"

extern "C"{
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
}

const char s_Tag[]  = "HTTP_OTA_UPDATE";
http_ota_update* http_ota_update::s_pInstance = NULL;
TaskHandle_t http_ota_update::xHandle = NULL;
uint8_t http_ota_update::mUpdateCounter = 0;

http_ota_update::http_ota_update()
{
    busy        = false;
    resp_data   = false;
    error       = false;
    hasPlanToUpdate = false;
    mUpdateCounter = 0;
    mLatitude = -1;
    mLongitude = -1;
    strcpy(sIp, "0.0.0.0");
    ESP_LOGD(s_Tag, "http_ota_update");
};

http_ota_update::~http_ota_update()
{
    ESP_LOGD(s_Tag, "~http_ota_update");
};

http_ota_update* http_ota_update::getInstance()
{
    if(s_pInstance == NULL)
    {
        s_pInstance = new http_ota_update();
    }
    return s_pInstance;
}

bool http_ota_update::check(uint8_t updateState)
{
    if(mLatitude == mLongitude && mLatitude == -1)
    {
        syncLocation();
    }

    busy        = true;
    error       = false;
    resp_data   = false;
    bool update = false;
    memset(resp_update, 0, SIZE_OF_BUFFER);

    char params[128];
    memset(params, 0, 128);
    char* sMac = wifi_sta::getInstance()->getMacAddress();
    sprintf(params, "hw_version=%s&sw_version=%s&sw_state=%u&device_mac_address=%s&counter=%d&ip=%s&latitude=%f&longitude=%f", 
            HARDWARE_VERSION, 
            SOFTWARE_VERSION, 
            updateState,
            sMac, mUpdateCounter,
            sIp, mLatitude, mLongitude);

    char update_url[512];
    memset(&update_url[0], 0, sizeof(update_url));
    sprintf(update_url, "%s?%s", UPDATE_API, params);

    http_request::get(update_url, "", [](const char* resp, size_t len) {
        http_ota_update* http_ota = http_ota_update::getInstance();
        if(len == 0)
        {
            http_ota->error = true;
            strcpy(http_ota->resp_update, "no_data");
        }
        else
        {
            strcpy(http_ota->resp_update, resp);
        }
        http_ota->resp_data = true;
    }, [](const char* resp, size_t len) {
        http_ota_update* http_ota = http_ota_update::getInstance();
        strcpy(http_ota->resp_update, resp);
        http_ota->error = true;
        http_ota->resp_data = true;
    }, 30);

    while(false == resp_data)
    {
        delay_ms(100);
    }

    if(false == error)
    {
        if(strstr(resp_update, "hardware_id") != NULL)
        {
            StaticJsonBuffer<512> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(resp_update);

            if (!root.success())
            {
                ESP_LOGD(s_Tag, "parseObject() failed");
                return update;
            }

            const char* update_version = root["update_version"];
            const char* update_description = root["update_description"];
            const char* update_url = root["update_url"];
            bool is_active = root["is_active"];
            uint8_t update_state = root["update_state"];
            uint64_t update_date = char2Uint64(root["update_date"]);

            strcpy(updateInfo.update_version, update_version);
            updateInfo.update_state = (SWUpdateMode)update_state;
            strcpy(updateInfo.update_description, update_description);
            strcpy(updateInfo.update_url, update_url);
            updateInfo.is_active = is_active;
            updateInfo.update_date = update_date;

            ESP_LOGD(s_Tag, "update_version: %s", updateInfo.update_version);
            ESP_LOGD(s_Tag, "update_state: %u", updateInfo.update_state);
            ESP_LOGD(s_Tag, "update_description: %s", updateInfo.update_description);
            ESP_LOGD(s_Tag, "update_url: %s", updateInfo.update_url);
            ESP_LOGD(s_Tag, "update_date1: %s" , uint642Char(updateInfo.update_date));

            if(strcmp(update_version, SOFTWARE_VERSION) > 0)  
            {  
                update = true;  
            }
        }
    }

    busy = false;
    return update;
};

/*
 * return true: update no error
 * return false: update error
 * */
bool http_ota_update::update()
{
    wifi_manager *cn = wifi_manager::getInstance();

    busy = true;
    http_request::getStream(updateInfo.update_url, "", [](const char *data, const size_t len){
        ota_update *ota = ota_update::getInstance();
        http_ota_update* http_ota = http_ota_update::getInstance();
        if(strcmp(data, "BEGIN") == 0 && 0 == len)
        {
            ESP_LOGD(s_Tag, "BEGIN");
            led_ctrl::setNotify();
            ota->begin();
        }
        else if(strcmp(data, "END") == 0 && 0 == len)
        {
            ESP_LOGD(s_Tag, "END");
            ota_update_result rs = ota->end();
            led_ctrl::setNotify();

            if(rs == OTA_OK)
            {
                ESP_LOGI(s_Tag, "OTA update successful");
            }
            else
            {
                sprintf(http_ota->resp_update, "update fail=%d", rs);
                http_ota->error = true;
            }
            sys_ctrl::setRunningMode(SYSTEM_MODE_OPERATION);
            delay_ms(1000);
            rebootSystem(0);
            http_ota->busy = false;
        }
        else
        {
            ota->writeHexData(data, len);
        }
    }, [](const char *error, const size_t len){
        http_ota_update* http_ota = http_ota_update::getInstance();
        sprintf(http_ota->resp_update, "update fail=%s", error);
        http_ota->error = true;
        http_ota->busy = false;
    });

    while(busy)
    {
        delay_ms(100);
    }

    ESP_LOGD(s_Tag, "%s", resp_update);

    return error;
};

void http_ota_update::autoUpdate(void* param)
{
    http_ota_update* instance = http_ota_update::getInstance();
    Config *cfg = Config::getInstance();
    wifi_sta* sta = wifi_sta::getInstance();
    uint8_t retry_update_count = 0;
    while(1)
    {
        if(true == sta->isConnected() && 
            false == instance->hasPlanToUpdate && 
            instance->check(cfg->getUpdateState()))
        {
            ESP_LOGI(s_Tag, "new version is available");
            if(SYSTEM_MODE_UPDATE == sys_ctrl::getRunningMode())
            {
                runone(NULL);
                sys_ctrl::setRunningMode(SYSTEM_MODE_OPERATION);
                rebootSystem();
                break;
            }
            else
            {
                sys_ctrl::setRunningMode(SYSTEM_MODE_UPDATE);
                rebootSystem();
            }
        }
        
        if(SYSTEM_MODE_UPDATE == sys_ctrl::getRunningMode())
        {
            retry_update_count++;
            if(retry_update_count > 2)
            {
                sys_ctrl::setRunningMode(SYSTEM_MODE_OPERATION);
                rebootSystem();
                break;
            }
            delay_ms(10000);
        }
        else
        {
            instance->mUpdateCounter++;
            delay_ms(TIME_INTERVAL_TO_CHECK_UDPATE_IN_MS);
            //reboot each 3 hours to protect system
            /*if(instance->mUpdateCounter > MAX_UPDATE_COUNTER) //3*4
            {
                rebootSystem(0);
            }*/
        }
    }
    vTaskDelete(NULL);
}

void http_ota_update::run()
{
    xTaskCreate(&http_ota_update::autoUpdate, "auto_update_ota", 8192, 
        NULL, tskIDLE_PRIORITY, &http_ota_update::xHandle);
}

void http_ota_update::runone(void* param)
{
    http_ota_update* instance = http_ota_update::getInstance();
    uint8_t retry = 0;
    const uint8_t retry_time_const = 3;
    while(retry < retry_time_const)
    {
        if(false == instance->update())
        {
            ++retry;
            delay_ms(10000);
            continue;
        }
        break;
    }
    instance->hasPlanToUpdate = false;
}

bool http_ota_update::syncLocation()
{
    busy        = true;
    error       = false;
    resp_data   = false;
    bool result = false;
    memset(resp_update, 0, SIZE_OF_BUFFER);

    char location_url[] = "http://35.192.136.53:8080/json/";

    http_request::get(location_url, "", [](const char* resp, size_t len) {
        http_ota_update* http_ota = http_ota_update::getInstance();
        if(len == 0)
        {
            http_ota->error = true;
            strcpy(http_ota->resp_update, "no_data");
        }
        else
        {
            strncpy(http_ota->resp_update, resp, len);
            ESP_LOGD(s_Tag, "syncLocation len=%d data=%s", len, resp);
        }
        http_ota->resp_data = true;
    }, [](const char* resp, size_t len) {
        http_ota_update* http_ota = http_ota_update::getInstance();
        strcpy(http_ota->resp_update, resp);
        http_ota->error = true;
        http_ota->resp_data = true;
    }, 30);

    while(false == resp_data)
    {
        delay_ms(100);
    }
    
    if(false == error && strstr(resp_update, "latitude") != NULL)
    {
        StaticJsonBuffer<512> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(resp_update);

        if (!root.success())
        {
            ESP_LOGD(s_Tag, "parseObject() failed");
            return result;
        }

        const char* ip = root["ip"];
        const float latitude = root["latitude"];
        const float longitude = root["longitude"];
        const char* timeZone = root["time_zone"];

        strcpy(sIp, ip);
        mLatitude = latitude;
        mLongitude = longitude;

        char timezone_url[] = "http://farmapi.agrhub.com/location/get";
        //sprintf(timezone_url, "http://api.timezonedb.com/v2.1/get-time-zone?key=ORXA5LMLOCF5&format=json&by=zone&zone=%s", timeZone);

        http_request::get(timezone_url, "", [](const char* resp, size_t len) {
            ESP_LOGD(s_Tag, "syncLocation len=%d data=%s", len, resp);
            if(len > 0 && resp != NULL && strstr(resp, "time_zone_offset") != NULL)
            {
                StaticJsonBuffer<1024> jsonBuffer;
                JsonObject& root = jsonBuffer.parseObject(resp);
                int gmtOffset = root["time_zone_offset"];
                gmtOffset *= -1;//convert negative
                //gmtOffset = 18000;

                float gmt = gmtOffset/3600;
                char timeZone[10] = "";
                sprintf(timeZone, "Etc/GMT%.1f", gmt);

                char strftime_buf[64];
                time_t now = 0;
                struct tm timeinfo;
                memset(&timeinfo, 0, sizeof(timeinfo));
                setenv("TZ", timeZone, 1);
                tzset();
                time(&now);
                localtime_r(&now, &timeinfo);
                strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
                ESP_LOGD(s_Tag, "syncLocation Local time=%s timeZone=%s gmtOffset=%d", strftime_buf, timeZone, gmtOffset);
            }
        }, [](const char* resp, size_t len) {
            
        }, 30);

        result = true;
    }

    busy = false;
    return result;
}



