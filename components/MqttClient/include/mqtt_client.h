#ifndef __MQTT_CLIENT_h__
#define __MQTT_CLIENT_h__

#include "config.h"

extern "C"
{
    #include "mongoose.h"
}

class mqtt_client
{
public:
    static const uint16_t MAX_TOPIC_LEN                = 128;
    static const uint32_t SIZE_OF_BUFFER               = 2048;
    static const uint32_t UPDATE_DATA_INTERVAL_TIME    = 60; //Connect to server every 60 seconds
    static const uint32_t SYNC_PROFILE_INTERVAL_TIME   = 5;
public:
    void start();
    static mqtt_client* getInstance();
    void sendMessage(const char* res);
private:
    mqtt_client();
    ~mqtt_client() {};

    static void eventHandler(struct mg_connection *nc, int ev, void *p);
    static void startTask(void * pvParameters);

    void handleMessage(const char* res, size_t len);

    static mqtt_client* s_pInstance;
    const static char s_Tag[];

public:
    static TaskHandle_t xHandle;
    static const char s_mqtt_server_broker[];
    static const char s_username[];
    static const char s_password[];
    char buffer[SIZE_OF_BUFFER];
    char topic_subscribe[MAX_TOPIC_LEN];
    char topic_publish[MAX_TOPIC_LEN];
    char app_name[MAX_TOPIC_LEN];
    bool close;
    bool connected;
    uint16_t mUpdateCounter;
    uint64_t mLastUpdate;
    mg_connection *nc;
};

#endif