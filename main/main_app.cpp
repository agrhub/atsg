/*
 * main.cpp
 *
 *  Created on: 18 thg 3, 2017
 *      Author: dmtan
 */

#include "wifi_manager.h"
#include "file_storage.h"
#include "atsg_connector.h"
#include "led_ctrl.h"
#include "mqtt_client.h"
#include "sys_ctrl.h"
#include "http_ota_update.h"
#include "uart_ctrl.h"
#include "esp_system.h"
#include "hang_timer_interrupt.h"
//60 seconds
#define HANG_TIMEOUT_CHECKING   60000000


extern "C"{
#include <stdio.h>
}
const char tag[]  = "Sense Hub 1.0";

extern "C" void app_main(void) {
    ESP_LOGI(tag, "SOFTWARE_VERSION=%s", SOFTWARE_VERSION);
    ESP_LOGI(tag, "SOFTWARE_DATE=%s", uint642Char(SOFTWARE_DATE));

    nvs_flash_init();
    led_ctrl::init();
    file_storage::init();
    sys_ctrl::init();

    //init watch dog trigger IO
    gpio_pad_select_gpio(WDT_PIN);
    gpio_set_direction(WDT_PIN, GPIO_MODE_OUTPUT);
    
    gpio_set_level(WDT_PIN, 1);
    delay_ms(1000);
    gpio_set_level(WDT_PIN, 0);

    if(sys_ctrl::getRunningMode() == SYSTEM_MODE_CONFIG)
    {
        led_ctrl::setMode(COLOR_GATEWAY_IN_CONFIG, LED_MODE_STATIC);
    }
    else
    {
        led_ctrl::setMode(COLOR_DISCONNECTED_WIFI, LED_MODE_BLINK);
    }

    //Prevent load old profile
    //profile::getInstance();

    wifi_manager::getInstance()->startService();
    init_prevent_hang_timer(HANG_TIMEOUT_CHECKING);
    
    switch(sys_ctrl::getRunningMode())
    {
        case SYSTEM_MODE_CONFIG:
        {
            ESP_LOGI(tag, "Running mode: SYSTEM_MODE_CONFIG");
            web_server::getInstance()->start();
            atsg_connector::getInstance();
            /*profile::getInstance()->load();
            device_manager::getInstance()->initSystem();
            web_server::getInstance()->start();*/
            break;
        }

        case SYSTEM_MODE_OPERATION:
        {
            ESP_LOGI(tag, "Running mode: SYSTEM_MODE_OPERATION");
            //web_server::getInstance()->start();
            uart_ctrl::getInstance();
            atsg_connector::getInstance();
            http_ota_update::getInstance()->run();
            mqtt_client::getInstance()->start();
            /*profile::getInstance()->load();
            device_manager::getInstance()->initSystem();
            web_server::getInstance()->start();*/
            //http_ota_update::getInstance()->run();
            //mqtt_client::getInstance()->start();
            //enhance prevent hang
            //init_prevent_hang_timer(HANG_TIMEOUT_CHECKING);
            break;
        }

        case SYSTEM_MODE_UPDATE:
        {
            ESP_LOGI(tag, "Running mode: SYSTEM_MODE_UPDATE");
            led_ctrl::setMode(COLOR_GATEWAY_IN_UPDATE, LED_MODE_BLINK);
            http_ota_update::getInstance()->run();
            break;
        }

        default:
        {
            ESP_LOGE(tag, "Code shoudn't touch here");
            break;
        }
    }
    ESP_LOGI(tag, "Free mem=%d", esp_get_free_heap_size());
};




