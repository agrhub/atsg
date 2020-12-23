/* udp_perf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#ifndef __UDP_CLIENT_H__
#define __UDP_CLIENT_H__

#include "config.h"

extern "C" {
#include <sys/socket.h>
}

#define MAX_BUFFER_LENGTH   1024

class UDPClient
{
public:
    esp_err_t send_data(const char* host, const uint32_t port, const void* payload, const size_t len, size_t& res_len);
    uint8_t* getBufferAddress();
    uint16_t getValidPort();
    ~UDPClient();
    void initSocket();

    static UDPClient* getInstance();
private:
    UDPClient();

private:
    uint8_t buffer[MAX_BUFFER_LENGTH];
    int sockfd;
    int serverlen;
    sockaddr_in serveraddr;

    static UDPClient* pUDPClientInstance;
    static const char mTag[];
    static SemaphoreHandle_t s_Semaphore;
};

#endif /*#ifndef __UDP_CLIENT_H__*/

