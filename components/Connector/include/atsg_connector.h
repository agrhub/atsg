/*
 * fiot_smart_tank.h
 *
 *  Created on: 24 thg 5, 2017
 *      Author: dmtan
 */

#ifndef COMPONENTS_SENSE_HYDRO_H_
#define COMPONENTS_SENSE_HYDRO_H_

#include "config.h"
//#include "connector.h"

extern "C"{
#include <stdio.h>
}

const char GET_CMD[]                   = "GET_CMD:%s\r";
//const char SET_CMD[]                   = "SET_CMD:%s\r";
const char SET_CMD[]                   = "%s\r";

class atsg_connector
{
private:
    bool runCMD(const char* command);
public:
    void sendData(const uint8_t* data, const size_t len=0);
    void getData(const uint8_t* data, const size_t len=0);
    void setData(const uint8_t* data, const size_t len=0);
    void handleData(const char* data);
private:
    char mac[32];
    char buf[1024];
private:
    atsg_connector();
    ~atsg_connector();
public:
    static atsg_connector* getInstance();
private:
    static atsg_connector* mInstance;
    static void createAndTakeSemaphore();
    static void releaseSemaphore();
    static SemaphoreHandle_t xSemaphore;
};

#endif /* COMPONENTS_SENSE_HYDRO_H_ */
