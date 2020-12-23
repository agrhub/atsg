#include "mqtt_client.h"
#include "wifi_sta.h"
#include "atsg_connector.h"
//#include "device_manager.h"
//#include "profile.h"
#define AUTO_TURN_OFF_TIMER 1*60*1000/portTICK_RATE_MS

mqtt_client* mqtt_client::s_pInstance = NULL;
TaskHandle_t mqtt_client::xHandle = NULL;
const char mqtt_client::s_Tag[] = "mqtt_client";
const char mqtt_client::s_mqtt_server_broker[] = "35.192.136.53:1883";
const char mqtt_client::s_username[] = "agrhub";
const char mqtt_client::s_password[] = "agrhub.com";

mqtt_client::mqtt_client()
{
    ESP_LOGD(s_Tag, "mqtt_client()");
    char* sMac = wifi_sta::getInstance()->getMacAddress();
    //char sMac[] = "30:AE:A4:1E:A3:C4";
    uint64_t ts = wifi_sta::getInstance()->getTimeStamp();
    if(strcmp(sMac, "") == 0)
    {
        ESP_LOGE(s_Tag, "Cannot get mac address");
    }
    else
    {
        memset(topic_subscribe, 0, MAX_TOPIC_LEN);
        memset(topic_publish, 0, MAX_TOPIC_LEN);
        sprintf(topic_subscribe, "to-gateway/%s", sMac);
        sprintf(topic_publish, "gateway/%s", sMac);
        sprintf(app_name, "ID_%s_%s", sMac, uint642Char(ts));
        ESP_LOGI(s_Tag, "Subscribing: %s", topic_subscribe);
        ESP_LOGI(s_Tag, "Publishing: %s", topic_publish);

        mUpdateCounter = 0;
        mLastUpdate = 0;
        close = true;
    }
}

mqtt_client* mqtt_client::getInstance()
{
    if(NULL == s_pInstance)
    {
        s_pInstance = new mqtt_client();
    }

    return s_pInstance;
}

void mqtt_client::eventHandler(struct mg_connection *nc, int ev, void *p)
{
    struct mg_mqtt_message *msg = (struct mg_mqtt_message *) p;

    mqtt_client* instance = mqtt_client::getInstance();
    instance->nc = nc;
    switch (ev)
    {
        case MG_EV_CONNECT:
        {
            int connect_status = * (int *) p;
            if (connect_status != 0)
            {
                ESP_LOGE(s_Tag, "Error: %d (%s)", connect_status, strerror(connect_status));
                break;
            }

            struct mg_send_mqtt_handshake_opts opts;
            memset(&opts, 0, sizeof(opts));

            opts.user_name = s_username;
            opts.password = s_password;
            opts.keep_alive = 0;
            mg_set_protocol_mqtt(nc);
            mg_send_mqtt_handshake_opt(nc, instance->app_name, opts);
            instance->mUpdateCounter = 0;
            instance->close = false;
            break;
        }
        case MG_EV_MQTT_CONNACK:
        {
            if (msg->connack_ret_code != MG_EV_MQTT_CONNACK_ACCEPTED)
            {
                ESP_LOGE(s_Tag, "Got mqtt connection error: %d", msg->connack_ret_code);
                break;
            }
            struct mg_mqtt_topic_expression topic_expr;
            topic_expr.qos = 0;
            topic_expr.topic = instance->topic_subscribe;

            ESP_LOGD(s_Tag, "Subscribing to %s", instance->topic_subscribe);
            mg_mqtt_subscribe(nc, &topic_expr, 1, 42);
          break;
        }

        case MG_EV_MQTT_PUBACK:
        {
            ESP_LOGD(s_Tag, "Message publishing acknowledged (msg_id: %d)", msg->message_id);
            break;
        }

        case MG_EV_MQTT_SUBACK:
        {
            ESP_LOGD(s_Tag, "Subscription acknowledged, forwarding to %s", instance->topic_publish);
            break;
        }

        case MG_EV_MQTT_PUBLISH:
        {
            ESP_LOGD(s_Tag, "Got incoming message %.*s: %.*s", (int) msg->topic.len,
                 msg->topic.p, (int) msg->payload.len, msg->payload.p);
            //ESP_LOGD(s_Tag, "Forwarding to %s", instance->topic_publish);
            //ESP_LOGD(s_Tag, "Protocal %d", msg->protocol_version);

            instance->handleMessage(msg->payload.p, msg->payload.len);
            break;
        }
        case MG_EV_CLOSE:
        {
            ESP_LOGE(s_Tag, "Connection closed");
            instance->close = true;
            instance->mUpdateCounter = 0;
            break;
        }

        case MG_EV_POLL:
        {
            if(instance->close == true)
            {
                instance->mUpdateCounter = 0;
                break;
            }

            uint64_t ts = wifi_sta::getInstance()->getTimeStamp();
            instance->mLastUpdate = ts;

            //get profile every 5mins
            /*if(instance->mUpdateCounter % (UPDATE_DATA_INTERVAL_TIME * SYNC_PROFILE_INTERVAL_TIME) == 0)
            {
                char* sMac = wifi_sta::getInstance()->getMacAddress();
                char data_to_send[128] = "";
                sprintf(data_to_send, "data={\"gateway_mac_address\":\"%s\", \"timestamp\":%s,\"mqtt_cmd\":\"sync_factory_profile\"}", 
                    sMac, uint642Char(ts));

                ESP_LOGD(s_Tag, "publishing data %d-%s", strlen(data_to_send), data_to_send);
                mg_mqtt_publish(nc, instance->topic_publish, 65, MG_MQTT_QOS(0), data_to_send,
                              strlen(data_to_send));
                delay_ms(1000);
            }

            //push data
            if(instance->mUpdateCounter % UPDATE_DATA_INTERVAL_TIME == 0)
            {
                char* data = device_manager::getInstance()->toJsonString(); //data=
                uint16_t len = strlen(data) + 10;
                char data_to_send[len];
                memset(data_to_send, '\0', len);
                strcat(data_to_send, "data=");
                strcat(data_to_send, data);

                ESP_LOGD(s_Tag, "publishing data %d-%s", strlen(data_to_send), data_to_send);
                mg_mqtt_publish(nc, instance->topic_publish, 65, MG_MQTT_QOS(0), data_to_send,
                              strlen(data_to_send));
            }

            if(instance->mUpdateCounter >= 10800)
            {
                instance->mUpdateCounter = 0;
            }
            else
            {
                ++instance->mUpdateCounter;  
            }*/

            break;
        }
        default:
        {
            break;
        }

  }
}

void mqtt_client::startTask(void * pvParameters)
{
    do
    {
        s_pInstance->connected = false;
        struct mg_mgr mgr;
        // wait wifi station is connected
        while(false == wifi_sta::getInstance()->isConnected())
        {
            uint64_t ts = wifi_sta::getInstance()->getTimeStamp();
            s_pInstance->mLastUpdate = ts;
            delay_ms(60000);
        }

        ESP_LOGD(s_Tag, "startTask tick 1");
        mg_mgr_init(&mgr, NULL);
        ESP_LOGI(s_Tag, "Mqtt client connect to %s", s_mqtt_server_broker);
        if (mg_connect(&mgr, s_mqtt_server_broker, eventHandler) == NULL)
        {
            ESP_LOGE(s_Tag, "mg_connect(%s) failed", s_mqtt_server_broker);
            delay_ms(60000);
        }
        else
        {
            //delay 3s before sending message
            delay_ms(3000);

            for(;;)
            {
                ESP_LOGD(s_Tag, "startTask tick 2 - free mem:%d", esp_get_free_heap_size());
                s_pInstance->connected = true;
                mg_mgr_poll(&mgr, 1000);
                if(true == s_pInstance->close)
                {
                    break;
                }
            }
        }
        
        //free memory
        mg_mgr_free(&mgr);
    } while(1);

    rebootSystem(0);

    vTaskDelete(NULL);
}

void mqtt_client::start()
{
    xTaskCreatePinnedToCore(mqtt_client::startTask, "mqtt_client", 8096, 
        NULL, tskIDLE_PRIORITY, &mqtt_client::xHandle, 1);
}

void mqtt_client::handleMessage(const char* res, size_t len)
{
    //formatJsonString(res, buffer, len);
    //ESP_LOGD(s_Tag, "Response len = %d res= %s", len, res);
    //fix crash issue
    char data[len+1] = "";
    strncpy(data, res, len);
    
    if(strstr(data, "device_mac_address") != NULL)
    {
        //parse json
        atsg_connector::getInstance()->handleData(data);
    }
}

void mqtt_client::sendMessage(const char* res)
{
    if(res != NULL && this->connected)
    {
        char data_to_send[256] = "";
        sprintf(data_to_send, "data=%s", res);

        ESP_LOGD(s_Tag, "publishing data %d-%s", strlen(data_to_send), data_to_send);
        mg_mqtt_publish(nc, topic_publish, 65, MG_MQTT_QOS(0), data_to_send,
                      strlen(data_to_send));
    }
}

