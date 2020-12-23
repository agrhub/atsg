/*
 * web_server.cpp
 *
 *  Created on: 27 thg 3, 2017
 *      Author: dmtan
 */

#include "web_server.h"
#include "wifi_scan.h"
#include "wifi_sta.h"
#include "config.h"
#include "mongoose.h"
#include "ota_update.h"
#include "http_ota_update.h"
#include "ArduinoJson.h"
//#include "device_manager.h"
#include "sys_ctrl.h"
#include "led_ctrl.h"
//#include "profile.h"


extern const uint8_t material_icons_svg_start[] asm("_binary_material_icons_svg_start");
extern const uint8_t material_icons_woff2_start[] asm("_binary_material_icons_woff2_start");

extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t logo_png_start[]   asm("_binary_logo_png_start");

extern const uint8_t jquery_js_start[] asm("_binary_jquery_min_js_start");
extern const uint8_t materialize_1_js_start[] asm("_binary_materialize_1_min_js_start");
extern const uint8_t materialize_2_js_start[] asm("_binary_materialize_2_min_js_start");
extern const uint8_t init_js_start[] asm("_binary_init_min_js_start");
extern const uint8_t device_js_start[] asm("_binary_device_min_js_start");
extern const uint8_t core_js_start[] asm("_binary_core_js_start");
extern const uint8_t lang_js_start[] asm("_binary_lang_min_js_start");

extern const uint8_t style_min_css_start[]   asm("_binary_style_min_css_start");

extern const uint8_t index_html_start[] asm("_binary_index_html_start");

extern const uint8_t material_icons_svg_end[] asm("_binary_material_icons_svg_end");
extern const uint8_t material_icons_woff2_end[] asm("_binary_material_icons_woff2_end");

extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");
extern const uint8_t logo_png_end[]   asm("_binary_logo_png_end");

extern const uint8_t jquery_js_end[] asm("_binary_jquery_min_js_end");
extern const uint8_t materialize_1_js_end[] asm("_binary_materialize_1_min_js_end");
extern const uint8_t materialize_2_js_end[] asm("_binary_materialize_2_min_js_end");
extern const uint8_t init_js_end[] asm("_binary_init_min_js_end");
extern const uint8_t device_js_end[] asm("_binary_device_min_js_end");
extern const uint8_t core_js_end[] asm("_binary_core_js_end");
extern const uint8_t lang_js_end[] asm("_binary_lang_min_js_end");

extern const uint8_t style_min_css_end[]   asm("_binary_style_min_css_end");

extern const uint8_t index_html_end[] asm("_binary_index_html_end");

static const struct mg_str GET = MG_MK_STR("GET");
static const struct mg_str POST = MG_MK_STR("POST");

const char web_server::s_Tag[] = "web_server";

web_server *web_server::s_pInstance = NULL;

static int is_method_equal(struct mg_str s1, struct mg_str s2)
{
    return s1.len == s2.len && memcmp(s1.p, s2.p, s2.len) == 0;
}


// // Convert a Mongoose string type to a string.
// const char *mgStrToStr(struct mg_str mgStr) {
//     char *retStr = new char[mgStr.len + 1];
//     strncpy(retStr, mgStr.p, mgStr.len);
//     retStr[mgStr.len] = '\0';
//     return retStr;
// } // mgStrToStr

void web_server::handleConnectWifi(struct mg_connection *nc, http_message *hm)
{
    char ssid[32], pwd[64];

    /* Get form variables */
    mg_get_http_var(&hm->body, "ssid", ssid, sizeof(ssid));
    mg_get_http_var(&hm->body, "pwd", pwd, sizeof(pwd));

    esp_wifi_disconnect();
    led_ctrl::setNotify();
    bool isConnected = wifi_sta::getInstance()->connectSTA(ssid, pwd, 10);

    /* Send headers */
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
    char res[20];
    memset(res, '\0', sizeof(res));
    sprintf(res, "{\"success\": %s}", wifi_sta::getInstance()->isConnected() ? "true" : "false");
    mg_send(nc, res, strlen(res));

    if(isConnected)
    {
        Config *cfg = Config::getInstance();
        cfg->writeCF("sta_ssid", ssid, CF_DATA_TYPE_CHAR);
        cfg->writeCF("sta_pwd", pwd, CF_DATA_TYPE_CHAR);

        sys_ctrl::setRunningMode(SYSTEM_MODE_OPERATION);
    }
}

void web_server::handleSetProductSerial(struct mg_connection *nc, http_message *hm)
{
    Config *cfg = Config::getInstance();
    if(is_method_equal(hm->method, POST))
    {
        char product_serial[32];
        char admin_password[32];
        bool result = false;

        /* Get form variables */
        mg_get_http_var(&hm->body, "admin_password", admin_password, sizeof(admin_password));
        mg_get_http_var(&hm->body, "product_serial", product_serial, sizeof(product_serial));

        if(strcmp(admin_password, "support@agrhub.com") == 0
                && strstr(product_serial, "SH-01-") != NULL //pattern SH-01-DDMMYY-XXXXXXX
                && strlen(product_serial) >= 20)
        {
            result = cfg->writeCF("product_serial", product_serial, CF_DATA_TYPE_CHAR);
            if(result)
            {
                cfg->setProductSerial(product_serial);
            }
        }
    }
    char html[512] = "";
    memset(html, '\0', sizeof(html));
    sprintf(html, "<html><body>"\
            "<form method='POST'>"\
            "<h4>Input admin password</h4>"\
            "<input type='password' name='admin_password'></input>"\
            "<h4>Input the product serial</h4>"\
            "<input type='text' name='product_serial' value='%s'></input>"\
            "<br/><br/><button type='submit'>SUBMIT</button></form></body></html>", cfg->getProductSerial());
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8 \r\n\r\n");
    mg_send(nc, html, strlen(html));

};

void web_server::handleOTAPage(struct mg_connection *nc, http_message *hm)
{
    char html[512] = "";
    memset(html, '\0', sizeof(html));
    strcpy(html, "<html><body>"\
            "<form method='POST' action=\"/ota_update\" enctype=\"multipart/form-data\">"\
            "<h4>Input admin password</h4>"\
            "<input type='password' name='admin_password'></input>"\
            "<h4>Input the product serial</h4>"\
            "<input type='file' name='file'></input>"\
            "<br/><br/><button type='submit'>SUBMIT</button></form></body></html>");
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8 \r\n\r\n");
    mg_send(nc, html, strlen(html));
};

void web_server::handleCheckNewFirmware(struct mg_connection *nc)
{
    http_ota_update *ota = http_ota_update::getInstance();
    Config *cfg = Config::getInstance();
    char resp[256] = "";
    if(true == ota->check(cfg->getUpdateState()))
    {
        sprintf(resp, "{\"update_version\" : \"%s\", \"update_url\" : \"%s\", \"update_date\" : %s}", ota->updateInfo.update_version, ota->updateInfo.update_url, uint642Char(ota->updateInfo.update_date));
    }
    else
    {
        sprintf(resp, "{\"update_version\" : \"%s\", \"update_url\" : \"%s\", \"update_date\" : %s}", ota->updateInfo.update_version, ota->updateInfo.update_url, uint642Char(ota->updateInfo.update_date));
    }

    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
    mg_send(nc, resp, strlen(resp));
}

void web_server::handleRunota_updateFirmware(struct mg_connection *nc, http_message *hm)
{
    char resp[128];

    http_ota_update *ota = http_ota_update::getInstance();
    bool rs = ota->update();
    if(rs)
    {
        sprintf(resp, "{\"success\": true}");
    }
    else{
        sprintf(resp, "{\"success\": false}");
    }
    ESP_LOGD(s_Tag, "resp=%s", resp);
    mg_printf(nc, "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
    mg_send(nc, resp, strlen(resp));
    nc->flags |= MG_F_SEND_AND_CLOSE;
}

void web_server::handleGetDeviceStats(struct mg_connection *nc)
{
    /*ESP_LOGD(s_Tag, "handleGetDeviceStats");
    device_manager *dm = device_manager::getInstance();
    char *json = dm->toJsonString();
    mg_printf(nc, "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
    mg_send(nc, json, strlen(json));*/
}

void web_server::handleSetSwitch(struct mg_connection *nc, http_message *hm)
{
    /*char device_state[10];
    char controller_type[5];
    mg_get_http_var(&hm->body, "device_state", device_state, sizeof(device_state));
    mg_get_http_var(&hm->body, "controller_type", controller_type, sizeof(controller_type));

    ESP_LOGD(s_Tag, "handleSetSwitch controller_type=%s device_state=%s", controller_type, device_state);
    device_manager *dev = device_manager::getInstance();
    if(NULL == dev)
    {
        ESP_LOGE(s_Tag, "Cannot found device");
        return;
    }    

    bool state = false;
    if(strcmp(device_state, "true") == 0)
    {
        state = true;
    }

    CONTROLLER_TYPE type = DEVICE_CMD_UNKNOW;
    type = (CONTROLLER_TYPE)atoi(controller_type);
    ESP_LOGD(s_Tag, "handleSetSwitch controller_type=%d device_state=%d", type, state);
    dev->setControllerState(type, state);

    char* json = dev->toJsonString();
    mg_printf(nc, "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
    mg_send(nc, json, strlen(json));*/
}

void web_server::handleSetIdle(struct mg_connection *nc, http_message *hm)
{
    /*char is_idle[10];
    mg_get_http_var(&hm->body, "is_idle", is_idle, sizeof(is_idle));

    ESP_LOGD(s_Tag, "handleSetIdle is_idle=%s", is_idle);
    device_manager *dev = device_manager::getInstance();
    if(NULL == dev)
    {
        ESP_LOGE(s_Tag, "Cannot found device");
        return;
    }    

    bool isIdle = false;
    if(strcmp(is_idle, "true") == 0)
    {
        isIdle = true;
    }
    //dev->setIdle(isIdle);

    char* json = dev->toJsonString();
    mg_printf(nc, "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
    mg_send(nc, json, strlen(json));*/
}

void web_server::handleNotFound(struct mg_connection *nc, http_message *hm)
{
    char content[] = "File not found";
    int len = sizeof(content);
    mg_send_head(nc, 404, len, "Content-Type: text/html");
    mg_send(nc, content, len);
};

void web_server::handleRebootDevice(struct mg_connection *nc, http_message *hm)
{
    /* Send headers */
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
    char res[20]= "{\"success\": true}";
    mg_send(nc, res, strlen(res));
    rebootSystem(10); // wait until fw reset
};

void web_server::handleSetting(struct mg_connection *nc, http_message *hm)
{
    /*ESP_LOGD(s_Tag, "web_server: handleSetting");
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
    char res[512] = "";
    profile* profile = profile::getInstance();
    device_manager *dev = device_manager::getInstance();
    if(is_method_equal(hm->method, POST))
    {
        char oldProfile[1024] = "";
        strcpy(oldProfile, profile->toJsonString());

        char ec_min[10] = "";
        char ec_max[10] = "";
        char ec_fomular_a[10] = "";
        char ec_fomular_b[10] = "";
        char ec_fomular_c[10] = "";
        char ec_fomular_d[10] = "";
        char ph_min[10] = "";
        char ph_max[10] = "";
        char water_pump_on_freq[5] = "";
        char water_pump_off_freq[5] = "";
        char water_level_min[5] = "";
        char water_level_max[5] = "";
        char tank_height[5] = "";
        char sensor_height[5] = "";
        
        int rs = mg_get_http_var(&hm->body, "ec_min", ec_min, sizeof(ec_min));
        mg_get_http_var(&hm->body, "ec_max", ec_max, sizeof(ec_max));
        if(rs > 0)
        {
            float min = atof(ec_min);
            float max = atof(ec_max);
            dev->setSensorThreshold(SENSOR_TYPE_WATER_EC, min, max);
        }
        
        rs = mg_get_http_var(&hm->body, "ec_fomular_a", ec_fomular_a, sizeof(ec_fomular_a));
        mg_get_http_var(&hm->body, "ec_fomular_b", ec_fomular_b, sizeof(ec_fomular_b));
        mg_get_http_var(&hm->body, "ec_fomular_c", ec_fomular_c, sizeof(ec_fomular_c));
        mg_get_http_var(&hm->body, "ec_fomular_d", ec_fomular_d, sizeof(ec_fomular_d));
        if(rs > 0)
        {
            uint8_t a = atoi(ec_fomular_a);
            uint8_t b = atoi(ec_fomular_b);
            uint8_t c = atoi(ec_fomular_c);
            uint8_t d = atoi(ec_fomular_d);
            dev->setECFomular(a, b, c, d);
        }
        
        rs = mg_get_http_var(&hm->body, "ph_min", ph_min, sizeof(ph_min));
        mg_get_http_var(&hm->body, "ph_max", ph_max, sizeof(ph_max));
        if(rs > 0)
        {
            float min = atof(ph_min);
            float max = atof(ph_max);
            dev->setSensorThreshold(SENSOR_TYPE_WATER_PH, min, max);
        }

        rs = mg_get_http_var(&hm->body, "water_pump_on_freq", water_pump_on_freq, sizeof(water_pump_on_freq));
        mg_get_http_var(&hm->body, "water_pump_off_freq", water_pump_off_freq, sizeof(water_pump_off_freq));
        if(rs > 0)
        {
            uint16_t on = atoi(water_pump_on_freq);
            uint16_t off = atoi(water_pump_off_freq);
            dev->setPumpFreq(on, off);
        }

        rs = mg_get_http_var(&hm->body, "water_level_min", water_level_min, sizeof(water_level_min));
        mg_get_http_var(&hm->body, "water_level_max", water_level_max, sizeof(water_level_max));
        mg_get_http_var(&hm->body, "tank_height", tank_height, sizeof(tank_height));
        mg_get_http_var(&hm->body, "sensor_height", sensor_height, sizeof(sensor_height));
        if(rs > 0)
        {
            uint8_t min = atoi(water_level_min);
            uint8_t max = atoi(water_level_max);
            uint16_t tankHeight = atoi(tank_height);
            uint16_t sensorHeight = atoi(sensor_height);
            dev->setSensorThreshold(SENSOR_TYPE_WATER_LEVEL, min, max);
            //profile->setSensorDistance(tankHeight, sensorHeight);
        }

        //commit
        strcpy(res, profile->toJsonString());
        if(strcmp(oldProfile, res) != 0)
        {
            profile->store();
        }
    }
    else
    {
        strcpy(res, profile->toJsonString());
    }
    mg_send(nc, res, strlen(res));*/
}

void web_server::handleContents(struct mg_connection *nc, http_message *hm, const char* uri)
{
    //prevent crash when many requests run at the same time
    ESP_LOGD(s_Tag, "handleContents - free mem=%d", esp_get_free_heap_size());
    if(strstr(uri, "index.html") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/html; charset=UTF-8\r\nKeep-Alive: timeout=15, max=100 \r\n\r\n");
        mg_send(nc, index_html_start, index_html_end - index_html_start);
    }
    else if(strstr(uri, "jquery.min.js") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; charset=UTF-8\r\nContent-Encoding: gzip\r\n\r\n");
        mg_send(nc, jquery_js_start, jquery_js_end - jquery_js_start);
    }
    else if(strstr(uri, "materialize.1.min.js") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; charset=UTF-8\r\nContent-Encoding: gzip\r\n\r\n");
        mg_send(nc, materialize_1_js_start, materialize_1_js_end - materialize_1_js_start);
    }
    else if(strstr(uri, "materialize.2.min.js") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; charset=UTF-8\r\nContent-Encoding: gzip\r\n\r\n");
        mg_send(nc, materialize_2_js_start, materialize_2_js_end - materialize_2_js_start);
    }
    else if(strstr(uri, "init.min.js") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; charset=UTF-8\r\nContent-Encoding: gzip\r\n\r\n");
        mg_send(nc, init_js_start, init_js_end - init_js_start);
    }
    else if(strstr(uri, "device.min.js") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; charset=UTF-8\r\nContent-Encoding: gzip\r\n\r\n");
        mg_send(nc, device_js_start, device_js_end - device_js_start);
    }
    else if(strstr(uri, "core.js") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; charset=UTF-8\r\n\r\n");
        mg_send(nc, core_js_start, core_js_end - core_js_start);
    }
    else if(strstr(uri, "lang.min.js") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; charset=UTF-8\r\nContent-Encoding: gzip\r\n\r\n");
        mg_send(nc, lang_js_start, lang_js_end - lang_js_start);
    }
    else if(strstr(uri, "style.min.css") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: text/css; charset=UTF-8\r\nContent-Encoding: gzip\r\n\r\n");
        mg_send(nc, style_min_css_start, style_min_css_end - style_min_css_start);
    }
    else if(strstr(uri, "material-icons.svg") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: image/svg+xml\r\n\r\n");
        mg_send(nc, material_icons_svg_start, material_icons_svg_end - material_icons_svg_start);
    }
    else if(strstr(uri, "material-icons.woff2") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/font-woff2\r\n\r\n");
        //fix webfont issue when read from esp32
        mg_send(nc, material_icons_woff2_start, material_icons_woff2_end - material_icons_woff2_start);

    }
    else if(strstr(uri, "logo.png") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\n\r\n");
        mg_send(nc, logo_png_start, logo_png_end - logo_png_start);
    }
    else if(strstr(uri, "favicon.ico") != NULL)
    {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\n\r\n");
        mg_send(nc, favicon_ico_start, favicon_ico_end - favicon_ico_start);
    }
    else
    {
        ESP_LOGE(s_Tag, "page not found %s", uri);
        handleNotFound(nc, hm);
    }

};

void web_server::handleScanWiFi(struct mg_connection *nc)
{
    ESP_LOGD(s_Tag, "handleScanWiFi - start");

    // WiFi.scanNetworks will return the number of networks found
    WiFiScanClass WiFi;
    int n = WiFi.scanNetworks();
    ESP_LOGD(s_Tag, "handleScanWiFi - found=%d", n);
    uint16_t max_length = (n+1)*128;
    char json[max_length];
    json[0] = '\0';
    strcpy(json,"[");

    for (int i = 0; i < n; i++)
    {
        // Print SSID and RSSI for each network found
        if(i > 0){
            strcat(json,",");
        }
        char buf[128];
        char* ssid = WiFi.SSID(i);
        sprintf(buf, "{\"name\":\"%s\",\"signal\":\"%d\",\"encrypted\":%s}",
                ssid, WiFi.RSSI(i), WiFi.encryptionType(i) ? "true" : "false");
        delete[] ssid;
        strcat(json, buf);
    }
    strcat(json,"]");

    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
    mg_send(nc, json, strlen(json));
};

void web_server::handleGetData(struct mg_connection *nc)
{
    ESP_LOGD(s_Tag, "handleGetData - start");
    Config *cfg = Config::getInstance();
    char* json = cfg->toJSString();
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
    mg_send(nc, json, strlen(json));
}

void web_server::handleGetSystemStatistic(struct mg_connection *nc)
{
//  ESP_LOGD(s_Tag, "handleGetData - start");
//  char json[512];
//  sprintf(json, "{\"free_mem\":%u, \"cpu_temperature\":%d, \"hall_sensor\":%d}",
//          esp_get_free_heap_size(), analog_temperature_read(), analog_hall_read());
//  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
//  mg_send(nc, json, strlen(json));
}

void web_server::handleSetMode(struct mg_connection *nc, http_message *hm)
{
    
    /*char mode[2];
    mg_get_http_var(&hm->body, "work_mode", mode, sizeof(mode));

    ESP_LOGD(s_Tag, "handleSetMode work_mode=%s", mode);
    if(strcmp(mode, "0") == 0)
    {
        sys_ctrl::setWorkingMode(SYSTEM_MODE_BRIDGE);
    }
    else if(strcmp(mode, "1") == 0)
    {
        sys_ctrl::setWorkingMode(SYSTEM_MODE_STANDALONE);
    }*/

    char res[] = "{\"success\": true}";
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
    mg_send(nc, res, strlen(res));

    //rebootSystem(10);
}

void web_server::handleFactoryReset(struct mg_connection *nc)
{
    ESP_LOGD(s_Tag, "handleFactoryReset");
    bool rs = sys_ctrl::resetFactory();
    char res[64];
    sprintf(res, "{\"success\": %s}", rs ? "true" : "false");
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
    mg_send(nc, res, strlen(res));
    if(rs)
    {
        rebootSystem(10);
    }
}

// Mongoose event handler.
void web_server::event_handler(struct mg_connection *nc, int ev, void *evData)
{
    uint64_t ts = wifi_sta::getInstance()->getTimeStamp();
    s_pInstance->mLastUpdate = ts;
    if(MG_EV_POLL != ev) ESP_LOGD(s_Tag, "ev=%d", ev)
    switch (ev)
    {
    case MG_EV_HTTP_REQUEST:
    {
        struct http_message *hm = (struct http_message *) evData;
        char *uri = new char[128];
        strncpy(uri, hm->uri.p, hm->uri.len);
        uri[hm->uri.len] = '\0';
        if(strcmp(uri, "/") == 0)
        {
            strcpy(uri, "/index.html");
        }
        else if(strcmp(uri, "/favicon.ico") == 0)
        {
            strcpy(uri, "/img/favicon.ico");
        }

        ESP_LOGD(s_Tag, "mongoose_event_handler uri=%s", uri);
        if(strstr(uri, ".html") != NULL
                || strstr(uri, ".js") != NULL
                || strstr(uri, ".css") != NULL
                || strstr(uri, ".woff2") != NULL
                || strstr(uri, ".ttf") != NULL
                || strstr(uri, ".png") != NULL
                || strstr(uri, ".ico") != NULL)
        {
            // mg_serve_http(nc, hm, opts);
            handleContents(nc, hm, uri);
        }
        else if(strstr(uri, "/update") != NULL)
        {
            if(is_method_equal(hm->method, GET))
            {
                handleOTAPage(nc, hm);
            }
        }
        else if(strstr(uri, "/api/v1/check_firmware") != NULL)
        {
            handleCheckNewFirmware(nc);
        }
        else if(strstr(uri, "/api/v1/run_ota_update_firmware") != NULL)
        {
            handleRunota_updateFirmware(nc, hm);
            ESP_LOGD(s_Tag, "Check point");
        }
        else if(strstr(uri, "/api/v1/get_data") != NULL)
        {
            handleGetData(nc);
        }
        else if(strstr(uri, "/api/v1/get_statistic") != NULL)
        {
            handleGetSystemStatistic(nc);
        }
        else if(strstr(uri, "/api/v1/scan_wifi") != NULL)
        {
            handleScanWiFi(nc);
        }
        else if(strstr(uri, "/api/v1/connect_wifi") != NULL)
        {
            if(is_method_equal(hm->method, POST))
            {
                handleConnectWifi(nc, hm);
            }
            else
            {
                mg_printf(nc, "%s",
                            "HTTP/1.0 501 Not Implemented\r\n"
                            "Content-Length: 0\r\n\r\n");
            }
        }
        else if(strstr(uri, "/api/v1/reboot_device") != NULL)
        {
            if(is_method_equal(hm->method, POST))
            {
                handleRebootDevice(nc, hm);
            }
            else
            {
                mg_printf(nc, "%s",
                            "HTTP/1.0 501 Not Implemented\r\n"
                            "Content-Length: 0\r\n\r\n");
            }
        }
        else if(strstr(uri, "/api/v1/product_serial") != NULL)
        {
            if(is_method_equal(hm->method, POST) || is_method_equal(hm->method, GET))
            {
                handleSetProductSerial(nc, hm);
            }
            else
            {
                mg_printf(nc, "%s",
                            "HTTP/1.0 501 Not Implemented\r\n"
                            "Content-Length: 0\r\n\r\n");
            }
        }
        else if(strstr(uri, "/api/v1/setting") != NULL)
        {
            if(is_method_equal(hm->method, POST) || is_method_equal(hm->method, GET))
            {
                handleSetting(nc, hm);
            }
            else
            {
                mg_printf(nc, "%s",
                            "HTTP/1.0 501 Not Implemented\r\n"
                            "Content-Length: 0\r\n\r\n");
            }
        }
        else if(strstr(uri, "/api/v1/devices") != NULL)
        {
            if(is_method_equal(hm->method, GET))
            {
                handleGetDeviceStats(nc);
            }
            else
            {
                mg_printf(nc, "%s",
                            "HTTP/1.0 501 Not Implemented\r\n"
                            "Content-Length: 0\r\n\r\n");
            }
        }
        else if(strstr(uri, "/api/v1/device") != NULL)
        {
            if(is_method_equal(hm->method, GET))
            {
                handleGetDeviceStats(nc);
            }
            if(is_method_equal(hm->method, POST))
            {
                handleSetSwitch(nc, hm);
            }
            else
            {
                mg_printf(nc, "%s",
                            "HTTP/1.0 501 Not Implemented\r\n"
                            "Content-Length: 0\r\n\r\n");
            }
        }
        else if(strstr(uri, "/api/v1/set_mode") != NULL)
        {
            if(is_method_equal(hm->method, POST))
            {
                handleSetMode(nc, hm);
            }
            else
            {
                mg_printf(nc, "%s",
                            "HTTP/1.0 501 Not Implemented\r\n"
                            "Content-Length: 0\r\n\r\n");
            }
        }
        else if(strstr(uri, "/api/v1/set_idle") != NULL)
        {
            if(is_method_equal(hm->method, POST))
            {
                handleSetIdle(nc, hm);
            }
            else
            {
                mg_printf(nc, "%s",
                            "HTTP/1.0 501 Not Implemented\r\n"
                            "Content-Length: 0\r\n\r\n");
            }
        }
        else if(strstr(uri, "/api/v1/factory_reset") != NULL)
        {
            if(is_method_equal(hm->method, POST))
            {
                handleFactoryReset(nc);
            }
            else
            {
                mg_printf(nc, "%s",
                            "HTTP/1.0 501 Not Implemented\r\n"
                            "Content-Length: 0\r\n\r\n");
            }
        }
        else
        {
            handleNotFound(nc, hm);
        }
        delete[] uri;
        nc->flags |= MG_F_SEND_AND_CLOSE;
        mbuf_remove(&nc->recv_mbuf, nc->recv_mbuf.len);
        break;
    }

    default:
        break;
    }

} // End of mongoose_event_handler

void web_server::handle_ota_update(struct mg_connection *nc, int ev, void *ev_data)
{
    ESP_LOGD(s_Tag, "handle_ota_update EV=%d", ev);
    struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) ev_data;

    ota_update *ota = ota_update::getInstance();

    switch (ev) {
    case MG_EV_HTTP_MULTIPART_REQUEST:
    {
        /* Get form variables */
        char adminPWD[32] = "";
        memset(adminPWD, '\0', sizeof(adminPWD));
        struct http_message *hm = (struct http_message *) ev_data;

        mg_get_http_var(&hm->query_string, "admin_password", adminPWD, sizeof(adminPWD));
        if(strcmp(adminPWD, "support@agrhub.com") != 0)
        {
            ESP_LOGD(s_Tag, "Check point 3");
            mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
            char res[] = "{\"success\": false}";
            mg_send(nc, res, strlen(res));
            nc->flags |= MG_F_SEND_AND_CLOSE;
            return;
        }
        led_ctrl::setNotify();
        ota->begin();

        break;
    }

    case MG_EV_HTTP_PART_DATA:
    {
        int len = mp->data.len;
        ota->writeHexData(mp->data.p, len);
        break;
    }
    case MG_EV_HTTP_MULTIPART_REQUEST_END:
    {
        ota_update_result rs = ota->end();

        if(rs == OTA_OK)
        {
            led_ctrl::setNotify();
            ESP_LOGI(s_Tag, "OTA update successful");
        }
        else
        {
            ESP_LOGD(s_Tag, "handle_ota_update fail=%d", rs);
        }

        mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8 \r\n\r\n");
        char res[20];
        memset(res, '\0', sizeof(res));
        sprintf(res, "{\"success\": %s}", rs == 0 ? "true" : "false");
        mg_send(nc, res, strlen(res));
        nc->flags |= MG_F_SEND_AND_CLOSE;
        mbuf_remove(&nc->recv_mbuf, nc->recv_mbuf.len);
        if(rs == OTA_OK)
        {
            rebootSystem(); // wait until fw reset
        }
        break;
    }
    default:
        break;
    }
}

// FreeRTOS task to start Mongoose.
void web_server::startTask(void *data)
{
    ESP_LOGD(s_Tag, "Mongoose task starting");
    struct mg_mgr mgr;
    mg_mgr_init(&mgr, NULL);

    ESP_LOGD(s_Tag, "Mongoose: Succesfully inited");
    mg_connection *cn = mg_bind(&mgr, ":80", event_handler);
    if (cn == NULL)
    {
        vTaskDelete(NULL);
        rebootSystem();
        return;
    }

    ESP_LOGD(s_Tag, "Mongoose Successfully bound");

    //handle OTA local update
    mg_register_http_endpoint(cn, "/ota_update", web_server::handle_ota_update);
    mg_set_protocol_http_websocket(cn);

    for(;;)
    {
        ESP_LOGD(s_Tag, "startTask tick");
        ESP_LOGD(s_Tag, "startTask - free mem=%d", esp_get_free_heap_size());
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);

    rebootSystem(0);
    vTaskDelete(NULL);
}

web_server::web_server()
{
    ESP_LOGD(s_Tag, "web_server");
    uint64_t ts = wifi_sta::getInstance()->getTimeStamp();
    mLastUpdate = ts;
}

web_server::~web_server()
{
    ESP_LOGD(s_Tag, "~web_server");
    s_pInstance = NULL;
}

web_server* web_server::getInstance()
{
    if(s_pInstance == NULL)
    {
        s_pInstance = new web_server();
    }
    return s_pInstance;
}

void web_server::start()
{
    xTaskCreatePinnedToCore(startTask, "web_server_task", 9126, NULL, tskIDLE_PRIORITY + 1, NULL, 1);
}