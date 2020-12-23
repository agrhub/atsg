/*
 * Update
 * Handle update
 *
 * Created: May 09, 2016
 * Author: Tan Do
 * Company: AgrHub - Bee Team
 * Email: dmtan@agrhub.com
 * Website: agrhub.com
 */

#ifndef UPDATE_h
#define UPDATE_h
#include "ota_update.h"
#include "config.h"

#include "wifi_manager.h"
#include "setting.h"

class http_ota_update
{
private:
    //static const uint16_t MAX_UPDATE_COUNTER                    = 12;
    static const uint16_t SIZE_OF_BUFFER                        = 512;
    static const uint32_t MINUTE_IN_MILISECONDS                 = 60 * 1000;
    static const uint32_t HOURS_IN_MILISECONDS                  = 60 * MINUTE_IN_MILISECONDS;
    static const uint32_t TIME_END_OF_DAY                       = 24 * HOURS_IN_MILISECONDS;
    static const uint32_t TIME_INTERVAL_TO_CHECK_UDPATE_IN_MS   = HOURS_IN_MILISECONDS/12; // each 5 minutes

    http_ota_update();
    ~http_ota_update();
    static void autoUpdate(void* param);

    static void runone(void* param);
public:
    bool check(uint8_t updateState);
    bool update();
    void run();
    bool syncLocation();

private:
    static http_ota_update * s_pInstance;

public:
    bool busy;
    bool resp_data;
    char resp_update[SIZE_OF_BUFFER];
    bool error;
    sw_update_struct updateInfo;
    bool hasPlanToUpdate;
    float mLatitude;
    float mLongitude;
    char sIp[20];
public:
    static http_ota_update* getInstance();
    static TaskHandle_t xHandle;
    static uint8_t mUpdateCounter;
};

#endif
