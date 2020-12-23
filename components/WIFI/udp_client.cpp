/* udp_perf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include "udp_client.h"
#include "config.h"

const char UDPClient::mTag[] = "UDP_CLIENT";

UDPClient* UDPClient::pUDPClientInstance = NULL;
SemaphoreHandle_t UDPClient::s_Semaphore = NULL;

UDPClient::UDPClient()
{
    ESP_LOGD(mTag, "UDPClient");
    sockfd = 0;
    initSocket();
}

UDPClient* UDPClient::getInstance()
{
    if(NULL == pUDPClientInstance)
    {
        pUDPClientInstance = new UDPClient();
    }
    return pUDPClientInstance;
}

uint8_t* UDPClient::getBufferAddress()
{
    return &buffer[0];
}

UDPClient::~UDPClient()
{
    ESP_LOGD(mTag, "~UDPClient");
    if (sockfd > 0)
    {
        close(sockfd);
    }

    pUDPClientInstance = NULL;
}

void UDPClient::initSocket()
{
    ESP_LOGD(mTag, "initSocket");
    if(NULL == s_Semaphore) 
    { 
        s_Semaphore = xSemaphoreCreateMutex(); 
    }

    while( xSemaphoreTake( s_Semaphore, ( TickType_t ) 100 ) == 0 );

    if (sockfd > 0)
    {
        close(sockfd);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        ESP_LOGE(mTag, "UDPClient error opening socket");
    }
    else
    {
        int enable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
        {
            ESP_LOGE(mTag, "setsockopt SO_REUSEADDR");
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(int)) == -1)
        {
            ESP_LOGE(mTag, "setsockopt SO_BROADCAST");
        }

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0) {
            ESP_LOGE(mTag, "setsockopt timeout failed");
        }
    }

    xSemaphoreGive( s_Semaphore );
}


/*
 * success return ESP_OK
 * fail return ESP_FAIL
 * */
esp_err_t UDPClient::send_data(const char* host, const uint32_t port, const void* payload,
        const size_t len, size_t& res_len)
{
    initSocket();
    
    if(NULL == s_Semaphore) 
    { 
        s_Semaphore = xSemaphoreCreateMutex(); 
    } 
 
    while( xSemaphoreTake( s_Semaphore, ( TickType_t ) 100 ) == 0 );

    esp_err_t status = ESP_FAIL;
    do
    {
        /* build the server's Internet address */
        sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        bzero(&(client_addr.sin_zero),sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = inet_addr(host);
        client_addr.sin_port = htons(port);

        ESP_LOGD(mTag, "send_data to %s:%d", host, port);
        uint8_t retry = 0;
        while(retry < 3)
        {
            if (-1 == sendto(sockfd, payload, len, 0, (sockaddr*)&client_addr, sizeof(client_addr)))
            {
                ESP_LOGD(mTag, "send_data sendto failed errno = %d", errno);
                ESP_LOGD(mTag, "send_data sendto retry = %d", retry+1);
                delay_ms(1000);
            }
            else
            {
                break;
            }
            retry++;
        }
        if(retry == 3)
        {
            break;
        }
        delay_ms(500);

        retry = 0;
        while(retry < 5)
        {
            res_len = recvfrom(sockfd, buffer, MAX_BUFFER_LENGTH, 0, (sockaddr*)&client_addr, (socklen_t*)&client_len);
            if (-1 == res_len)
            {
                ESP_LOGD(mTag, "send_data recvfrom failed");
                delay_ms(1000);
            }
            else
            {
                break;
            }
            retry++;
        }
        if(retry < 5)
        {
            status = ESP_OK;
        }
    } while (0);
    shutdown(sockfd, SHUT_RD);
    xSemaphoreGive( s_Semaphore );
    return status;
}

uint16_t UDPClient::getValidPort()
{
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
    {
        ESP_LOGD(mTag, "getValidPort failed");
        return 0;
    }
    uint16_t chosenPort = ntohs(sin.sin_port);

    return chosenPort;
}