#include "hang_timer_interrupt.h"
#include "config.h"
#include "wifi_sta.h"
#include "sys_ctrl.h"
#include "mqtt_client.h"
#include "http_ota_update.h"
//60 minutes
#define REBOOT_SYSTEM_TIMEOUT 60*60*1000
#define REBOOT_MAX_COUNTER_CONFIG_MODE      30//30 MINUTES
#define REBOOT_MAX_COUNTER_RUNNING_MODE     3*60//3 HOURS
#define REBOOT_MAX_COUNTER_UPDATING_MODE    30//30 MINUTES
static uint16_t time_counter   = 0;

static void hang_checking(void * pvParameters)
{
    ESP_LOGD("SYSTEM", "Gateway hang checking");
    gpio_set_level(WDT_PIN, 1);
    delay_ms(1000);
    gpio_set_level(WDT_PIN, 0);
    time_counter++;
    bool isRebooted = false;
    if(sys_ctrl::getRunningMode() == SYSTEM_MODE_CONFIG 
        && time_counter >= REBOOT_MAX_COUNTER_CONFIG_MODE)
    {
        isRebooted = true;
        //force to running mode when configuration timeout
        Config *cfg = Config::getInstance();
        char* ssid = cfg->getSTASSID();
        char* pwd = cfg->getSTAPWD();
        if(strlen(ssid) > 0)
        {
            sys_ctrl::setRunningMode(SYSTEM_MODE_OPERATION);
            delay_ms(1000);
        }
    }
    else if(sys_ctrl::getRunningMode() == SYSTEM_MODE_OPERATION 
        && time_counter >= REBOOT_MAX_COUNTER_RUNNING_MODE)
    {
        isRebooted = true;
    }
    else if(sys_ctrl::getRunningMode() == SYSTEM_MODE_UPDATE 
        && time_counter >= REBOOT_MAX_COUNTER_UPDATING_MODE)
    {
        isRebooted = true;
    }
    
    if(sys_ctrl::getRunningMode() == SYSTEM_MODE_OPERATION)
    {
        //bool isConnected = wifi_sta::getInstance()->isConnected();
        uint64_t ts = wifi_sta::getInstance()->getTimeStamp();
        ts = ts - mqtt_client::getInstance()->mLastUpdate;
        //restart if not update over 5 minutes
        if(mqtt_client::getInstance()->mLastUpdate > 0 && 
            ts > REBOOT_SYSTEM_TIMEOUT && 
            http_ota_update::getInstance()->mUpdateCounter > 0)
        {
            isRebooted = true;
        }
    }

    if(isRebooted == true)
    {
        rebootSystem(0);
    }

    vTaskDelete(NULL);
}

void IRAM_ATTR timer_prevent_hang_isr(void* arg)
{
    TIMERG0.int_clr_timers.t0 = 1;
    TIMERG0.hw_timer[0].config.alarm_en = 1;
    xTaskCreate(hang_checking, "Gateway hang checking", 2048, NULL, tskIDLE_PRIORITY, NULL);
};
