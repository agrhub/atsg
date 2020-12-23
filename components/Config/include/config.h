/*
 * Access point connection
 * Handle access point connection mode
 *
 * Created: Jan 26, 2016
 * Author: Tan Do
 * Company: BeeIO
 * Email: dmtan90@gmail.com
 * Website: beeiovn.com
 */

#ifndef CONFIG_h
#define CONFIG_h
#include "setting.h"

extern "C" {
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
#include <freertos/timers.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_partition.h>
#include <nvs.h>
#include <driver/gpio.h>
}

#ifndef DB_CONFIG
#define DB_CONFIG

#define COLOR_DISCONNECTED_WIFI     0x000020
#define COLOR_CONNECTED_WIFI        0x002000
#define COLOR_DEVICE_CONNECTING     0x070b0f
#define COLOR_GATEWAY_IN_CONFIG     0x200000
#define COLOR_GATEWAY_IN_UPDATE     0x8e24aa

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

#define WATER_LEVEL_TXD_PIN (GPIO_NUM_14)
#define WATER_LEVEL_RXD_PIN (GPIO_NUM_12)

#define LED_PIN (GPIO_NUM_18)

#define SW_PIN (GPIO_NUM_0)

#define RELAY_1_PIN (GPIO_NUM_15)
#define RELAY_2_PIN (GPIO_NUM_16)
#define RELAY_3_PIN (GPIO_NUM_17)
#define RELAY_4_PIN (GPIO_NUM_19)

#define WDT_PIN (GPIO_NUM_21)

#define RGB_RED_PIN (GPIO_NUM_25)
#define RGB_GREEN_PIN (GPIO_NUM_26)
#define RGB_BLUE_PIN (GPIO_NUM_27)

static const uint32_t WARNING_FREE_MEM  = 70000;
static const uint32_t CRITICAL_FREE_MEM = 50000;

enum DB_OPEN_MODE{
    READ_ONLY   = 1,
    READ_WRITE  = 2
};

enum CONTROLLER_STATE {
    CONTROLLER_STATE_ON,
    CONTROLLER_STATE_OFF,
    CONTROLLER_STATE_UNKNOWN
};

enum CONTROLLER_CHANNEL {
    CONTROLLER_CHANNEL_1 = 0,
    CONTROLLER_CHANNEL_2 = 1,
    CONTROLLER_CHANNEL_3 = 2,
    CONTROLLER_CHANNEL_4 = 3,
    CONTROLLER_CHANNEL_COUNT
};

enum CONTROLLER_TYPE {
    DEVICE_CMD_UNKNOW                   = 0,
    DEVICE_CMD_LAMP                     = 1,
    DEVICE_CMD_PUMP                     = 2,
    DEVICE_CMD_MISTING                  = 3,
    DEVICE_CMD_FAN                      = 4,
    DEVICE_CMD_AIR_CONDITIONER          = 5,
    DEVICE_CMD_CO2_CONTROLLER           = 6,
    DEVICE_CMD_DOSING_PUMP_CONTROLLER   = 7,
    DEVICE_CMD_OXYGEN_PUMP_CONTROLLER   = 8,
    DEVICE_CMD_VALVE_INPUT             	= 9,
    DEVICE_CMD_VALVE_OUTPUT            	= 10,
	DEVICE_CMD_HYDRO_TANK_WASHING 		= 11
};

enum DB_DEVICE_TYPE {
    DB_DEVICE_TYPE_UNKNOWN          = 0,
    DB_DEVICE_TYPE_SENSOR           = 1,
    DB_DEVICE_TYPE_CONTROLLER       = 2,
    DB_DEVICE_TYPE_CONTAINER        = 3,
    DB_DEVICE_TYPE_GATEWAY          = 4
};

enum DB_DEVICE_NAME {
    DB_DEVICE_NAME_UNKNOWN                  =   0,
    DB_DEVICE_NAME_SP3_SMART_PLUG           =   1,
    DB_DEVICE_NAME_SONOFF_SMART_PLUG        =   2,
    DB_DEVICE_NAME_XIAOMI_SMART_PLUG        =   3,
    DB_DEVICE_NAME_MI_FLORA                 =   4,
    DB_DEVICE_NAME_AXEC_AIR_SENSOR          =   5,
    DB_DEVICE_NAME_TI_TAG_SENSOR            =   6,
    DB_DEVICE_NAME_FIOT_SMART_TANK          =   7,
    DB_DEVICE_NAME_RM3_SMART_REMOTE         =   8,
    DB_DEVICE_NAME_GATEWAY                  =   9,
    DB_DEVICE_NAME_VIETTEL_CAMERA           =   10,
    DB_DEVICE_NAME_SENSE_HUB_PRO            =   11,
    DB_DEVICE_NAME_NRF51822_SENSOR          =   12,
    DB_DEVICE_NAME_VGV_CAMERA               =   13,
    DB_DEVICE_NAME_MI_AIR_SENSOR            =   14,
    DB_DEVICE_NAME_MI_POT                   =   15,
    DB_DEVICE_NAME_KPL_AIR_SENSOR           =   16,
    DB_DEVICE_NAME_KPL_FLOOD_SENSOR         =   17,
    DB_DEVICE_NAME_KAMOER_DRIPPING_PRO      =   18,
    DB_DEVICE_NAME_RTMP_CAMERA              =   19,
    DB_DEVICE_NAME_THUNDERBOARD_SENSE       =   20,
    DB_DEVICE_NAME_ARIATEC_SMART_TANK       =   21,
    DB_DEVICE_NAME_SENSE_AC                 =   22,
    DB_DEVICE_NAME_SENSE_PLUG               =   23,
    DB_DEVICE_NAME_JENCO_PH610B             =   24,
    DB_DEVICE_NAME_SENSE_WATER_LEVEL        =   25,
    DB_DEVICE_NAME_POMETER_PH_260BD         =   26,
    DB_DEVICE_NAME_ESL_TAG                  =   27,
    DB_DEVICE_NAME_E_SENSE                  =   28,
    DB_DEVICE_VCV_CAMERA_2                  =   29,
    DB_DEVICE_ARIATEC_SMART_TANK_CHILLER_HEATER_CONTROLLER    =   30,
    DB_DEVICE_SENSE_HYDRO_HOME_CONTROLLER             =   31,
    DB_DEVICE_SENSE_HYDRO_HOME_WIFI_CONTROLLER        =   32,
    DB_DEVICE_ATSG_CONTROLLER               =   33,
    DB_DEVICE_NAME_COUNT
};

enum SENSOR_TYPE{
    SENSOR_TYPE_UNKNOWN         	= 0,
    SENSOR_TYPE_LIGHT           	= 1,
    SENSOR_TYPE_AIR_TEMP        	= 2,
    SENSOR_TYPE_AIR_HUMIDITY    	= 3,
    SENSOR_TYPE_SOIL_TEMP       	= 4,
    SENSOR_TYPE_SOIL_HUMIDITY   	= 5,
    SENSOR_TYPE_SOIL_EC         	= 6,
    SENSOR_TYPE_SOIL_PH         	= 7,
    SENSOR_TYPE_WATER_TEMP      	= 8,
    SENSOR_TYPE_WATER_EC        	= 9,
    SENSOR_TYPE_WATER_PH        	= 10,
    SENSOR_TYPE_WATER_ORP       	= 11,
    SENSOR_TYPE_BAT             	= 12,
    SENSOR_TYPE_CO2             	= 13,
	SENSOR_TYPE_WATER_LEVEL			= 14,
	SENSOR_TYPE_WATER_DETECT		= 15,
	SENSOR_TYPE_ERROR_DETECT		= 16,
    SENSOR_TYPE_UV                  = 17,
    SENSOR_TYPE_PRESSURE            = 18,
    SENSOR_TYPE_SOUND               = 19,
    SENSOR_TYPE_VOC                 = 20,
    SENSOR_TYPE_ACCELERATOR         = 21,
    SENSOR_TYPE_ORIENTATION         = 22,
    SENSOR_TYPE_HALL                = 23
};

enum DEVICE_STATE{
    DEVICE_CONNECTED                = 0,
    DEVICE_DISCONNECTED             = 1,
    DEVICE_ERROR                    = 2
};

enum EntityFieldType{
    ENTITY_FIELD_CHAR       = 0,
    ENTITY_FIELD_BOOL       = 1,
    ENTITY_FIELD_INT8_T     = 2,
    ENTITY_FIELD_UINT8_T    = 3,
    ENTITY_FIELD_INT16_T    = 4,
    ENTITY_FIELD_UINT16_T   = 5,
    ENTITY_FIELD_INT32_T    = 6,
    ENTITY_FIELD_UINT32_T   = 7,
    ENTITY_FIELD_INT64_T    = 8,
    ENTITY_FIELD_UINT64_T   = 9,
    ENTITY_FIELD_DOUBLE     = 10
};

enum LED_MODE
{
    LED_MODE_STATIC,
    LED_MODE_BLINK,
    LED_MODE_NOTIFY,
    LED_MODE_COUNT
};

enum RUNNING_MODE_CONFIG
{
    SYSTEM_MODE_CONFIG = 0,
    SYSTEM_MODE_OPERATION,
    SYSTEM_MODE_UPDATE
};

/*enum WORKING_MODE_CONFIG
{
    SYSTEM_MODE_BRIDGE = 0, //connect with Sense Hub gateway
    SYSTEM_MODE_STANDALONE //connect direct to MQTT server
};*/

struct EntityField{
    char name[32]="";
    void* value = NULL;
    EntityFieldType type = ENTITY_FIELD_CHAR;
};


#endif

enum CFDataType{
    CF_DATA_TYPE_CHAR   = 0,
    CF_DATA_TYPE_INT8   = 1,
    CF_DATA_TYPE_UINT8  = 2,
    CF_DATA_TYPE_INT16  = 3,
    CF_DATA_TYPE_UINT16 = 4,
    CF_DATA_TYPE_INT32  = 5,
    CF_DATA_TYPE_UINT32 = 6,
    CF_DATA_TYPE_INT64  = 7,
    CF_DATA_TYPE_UINT64 = 8,
    CF_DATA_TYPE_FLOAT  = 9
};

enum KamoerControlMode{
    KAMOER_IDLE_STATE       = 0,
    KAMOER_GET_STATE        = 1,
    KAMOER_SET_STATE        = 2
};

struct ConfigStruct{
    char company_name[32] = COMPANY_NAME;
    char product_name[32] = PRODUCT_NAME;
    char product_serial[32] = "";
    uint64_t product_release_date = PRODUCT_RELEASE_DATE;

    char hw_version[10] = HARDWARE_VERSION;
    char sw_version[10] = SOFTWARE_VERSION;
    uint64_t sw_date = SOFTWARE_DATE;

    char ap_ssid[33] = AP_SSID;
    char ap_pwd[33] = AP_PWD;
    char sta_ssid[33] = "";
    char sta_pwd[33] = "";

    uint8_t boot_mode = SYSTEM_MODE_CONFIG;

    SWUpdateMode update_state = SW_UPDATE_MODE_STABLE;

    float ec = 1.0;
    float ph = 6.5;
};


#ifndef REBOOT_STASK
#define REBOOT_STASK

#define delay_ms(ts) vTaskDelay(ts/portTICK_RATE_MS)

static uint64_t char2Uint64(const char *str)
{
  uint64_t result = 0; // Initialize result
  // Iterate through all characters of input string and update result
  for (int i = 0; str[i] != '\0'; ++i){
    result = result*10 + str[i] - '0';
  }
  return result;
};

static char uint642CharBuf[20] = "";
static char* uint642Char(const uint64_t num)
{
    uint64_t temp = num;
  char buf[20];
  buf[0] = '\0';
  //memset(buf, '\0', 20);

  if (temp == 0)
  {
    buf[0] = '0';
  }

  while (temp > 0)
  {
    uint64_t q = temp/10;
    uint8_t val = temp - q*10;
    char t[2];
    sprintf(t, "%d", val);
    strcat(buf, t);
    temp = q;
  }
  int len = strlen(buf);
  for(int i = 0; i < len/2;i++)
  {
    char c = buf[i];
    buf[i] = buf[len - i - 1];
    buf[len - i - 1] = c;
  }
  uint642CharBuf[0] = '\0';
  strcpy(uint642CharBuf, buf);
  return uint642CharBuf;
};

static uint16_t sRebootTimer = 0;

static void rebootTask(void* data)
{
    while(sRebootTimer)
    {
        ESP_LOGI("System", "Reboot in %d seconds...", sRebootTimer);
        delay_ms(1000);
        --sRebootTimer;
    }

    esp_restart();
    vTaskDelete(NULL);
}

static void rebootSystem(uint16_t timer_counter = 3)
{
    ESP_LOGI("System", "rebootSystem in %d", timer_counter);
    if(timer_counter > 0)
    {
        sRebootTimer = timer_counter;
        xTaskCreate(rebootTask, "rebootTask", 1024, NULL, tskIDLE_PRIORITY, NULL);
    }
    else
    {
        ESP_LOGI("System", "Reboot system now");
        esp_restart();
    }
}


#endif

class Config{
  public:
    Config();

    ~Config();

    void* readCF(const char* name, CFDataType type, size_t len = 128);

    bool writeCF(const char* name, const void* value, CFDataType type);

    bool restore();

    char* toJSString();

    char* getAPSSID();

    void setAPSSID(const char* ssid);

    char* getAPPWD();

    void setAPPWD(const char* pwd);

    char* getSTASSID();

    void setSTASSID(const char* ssid);

    char* getSTAPWD();

    void setSTAPWD(const char* pwd);

    RUNNING_MODE_CONFIG getBootMode();

    void setBootMode(RUNNING_MODE_CONFIG mode);

    char* getCompanyName();

    char* getProductName();

    void setProductName(const char* name);

    char* getProductSerial();

    void setProductSerial(const char* serial);

    uint64_t getProductReleaseDate();

    char* getHardwareVersion();

    char* getSoftwareVersion();

    uint64_t getSWUpdatedDate();

    SWUpdateMode getUpdateState();

    void setUpdateState(SWUpdateMode state);

    float getEC();
    void setEC(float ec);

    float getPH();
    void setPH(float ph);

    void commitSetpoints();

    static Config* getInstance();

  private:
    static void createAndTakeSemaphore();
    static void releaseSemaphore();
    
    ConfigStruct cfg;
    char buffer[512];

    static SemaphoreHandle_t xSemaphore;
};

#endif
