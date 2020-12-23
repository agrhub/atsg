#include "uart_ctrl.h"
#include "config.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "atsg_connector.h"
#include <time.h>       /* time */

const char uart_ctrl::mTag[] = "uart_ctrl";
uart_ctrl* uart_ctrl::s_pInstance = NULL;
SemaphoreHandle_t uart_ctrl::xSemaphore = NULL;

float RandomFloat(float min, float max) 
{
	/* initialize random seed: */
  	srand (time(NULL));
    // this  function assumes max > min, you may want 
    // more robust error checking for a non-debug build
    assert(max > min); 
    float random = ((float) rand()) / (float) RAND_MAX;

    // generate (in your case) a float between 0 and (4.5-.78)
    // then add .78, giving you a float between .78 and 4.5
    float range = max - min;  
    return (random*range) + min;
}

uint16_t simulateUartData(uint8_t* data, uint16_t buffSize)
{
	if(data == NULL || buffSize == 0)
	{
		return 0;
	}
	memset(data, 0x00, buffSize);

	uart_ctrl* uart = uart_ctrl::getInstance();
	char cmd[64] = "";
	strcpy(cmd, uart->mLastCMD);
	if(strcmp(cmd, "") == 0)
	{
		return 0;
	}

	ESP_LOGD("uart_ctrl", "simulateUartData - cmd: %s", cmd);
	char * pch;
	char code[2] = "";
	char tmp[5] = "";
	uint16_t p1 = 0;
	uint16_t p2 = 0;
	uint16_t p3 = 0;

	pch = strtok (cmd, ";");
	int i = 0;
	while (pch != NULL)
	{
		if(i == 0)
		{
			strcpy(code, pch);
		}
		else if(i == 1)
		{
			strcpy(tmp, pch);
			p1 = atoi(tmp);
		}
		else if(i == 2)
		{
			strcpy(tmp, pch);
			p2 = atoi(tmp);
		}
		else if(i == 3)
		{
			strcpy(tmp, pch);
			p3 = atoi(tmp);
		}

		pch = strtok (NULL, ";");
		i++;
	}

	char temp[buffSize] = "";

	if(strcmp(code, "v") == 0)
	{
		if(p3 > 0)
		{
			//delay_ms(60000);
			p3 -= 1;

		}
		sprintf(temp, "%s;%d;%d;%d", code, p1, p2, p3);
	}
	
	ESP_LOGD("uart_ctrl", "simulateUartData: '%s'", temp);

	int len = strlen(temp);
	for(int i = 0; i < len; i++)
	{
		data[i] = temp[i];
	}

	strcpy(uart->mLastCMD, "");

	return len;
}

void uart_ctrl::rx_task(void* param)
{
    ESP_LOGD(mTag, "rx_task");
    uint8_t* data = new uint8_t[uart_ctrl::RX_BUF_SIZE];
    while(1)
    {
    	/*if(s_pInstance->isWaitingData == true)
    	{
    		uint8_t coundown = 30;
		    uint8_t* data = new uint8_t[uart_ctrl::RX_BUF_SIZE];
		    while (coundown > 0) 
		    {
		    	//simulate UART data
		    	//const int rxBytes = simulateUartData(data, uart_ctrl::RX_BUF_SIZE);
		        const int rxBytes = uart_read_bytes(UART_NUM_1, data, uart_ctrl::RX_BUF_SIZE, 200 / portTICK_RATE_MS);
		        if (rxBytes > 0) 
		        {
		            data[rxBytes] = 0;
		            ESP_LOGD(mTag, "Read %d bytes: '%s'", rxBytes, data);
		            s_pInstance->setData(data, rxBytes);
		            break;
		        }
		        delay_ms(100);
		        coundown--;
		    }
		    delete[] data;
    	}*/
    	//const int rxBytes = simulateUartData(data, uart_ctrl::RX_BUF_SIZE);
    	const size_t rxBytes = uart_read_bytes(UART_NUM_1, data, uart_ctrl::RX_BUF_SIZE, 100 / portTICK_RATE_MS);
        if (rxBytes > 0) 
        {
            data[rxBytes] = 0;
            ESP_LOGD(mTag, "Read %d bytes: '%s'", rxBytes, data);
            s_pInstance->setData(data, rxBytes);
            //break;
        }
	    delay_ms(10);
    }
    delete[] data;
    rebootSystem(0);
    vTaskDelete(NULL);
}

uart_ctrl::uart_ctrl()
{
	ESP_LOGD(mTag, "uart_ctrl");
	for(int i = 0; i < RX_BUF_SIZE; i++)
	{
		mData[i] = 0x00;
	}
	mLength = 0;
	isWaitingData = false;
	init();
};

uart_ctrl::~uart_ctrl()
{
	ESP_LOGD(mTag, "~uart_ctrl");	
}

uart_ctrl* uart_ctrl::getInstance()
{
	//ESP_LOGD(mTag, "getInstance");
	if(s_pInstance == NULL)
	{
		s_pInstance = new uart_ctrl();
	}
	return s_pInstance;
}


void uart_ctrl::init()
{
	ESP_LOGD(mTag, "init");
	const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    //init receive response data
    xTaskCreate(rx_task, "UART Communication", 4096, NULL, tskIDLE_PRIORITY, NULL);
}

bool uart_ctrl::sendData(const char* data, bool needResponse)
{
	createAndTakeSemaphore();
	bool success = false;
	const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGD(mTag, "sendData: Wrote %d bytes", txBytes);
    if(len == txBytes)
    {
    	success = true;
    	strcpy(mLastCMD, data);
    }

    //syncronize
	/*if(success == true && needResponse == true)
	{
		//clear old data
		this->isWaitingData = true;
		this->setData(NULL);
		uint8_t countdown = 30;
		while(mLength == 0 && countdown > 0)
		{
			delay_ms(50);
			countdown--;
		}

		if(mLength == 0)
		{
			success = false;
		}
		this->isWaitingData = false;
	}*/
	releaseSemaphore();
	return success;
}

void uart_ctrl::setData(const uint8_t* data, const size_t length)
{
	/*if(strncmp((char*)data, (char*)mData, length) == 0)
	{
		return;
	}*/

	for(int i = 0; i < RX_BUF_SIZE; i++)
	{
		mData[i] = 0x00;
	}

	//char tmp[length+1] = "";
	if(length > 0 && data != NULL)
	{
		mLength = length;
		for(int i = 0; i < length; i++)
		{
			mData[i] = data[i];
			//tmp[i] = data[i];
		}
	}
	else
	{
		mLength = 0;
	}
	
	if(data != NULL && length > 0)
	{
		atsg_connector* conn = atsg_connector::getInstance();
		conn->getData(mData, mLength);	
	}
}

uint8_t* uart_ctrl::readData(uint16_t &len)
{
	len = mLength;
	return mData;
}

void uart_ctrl::createAndTakeSemaphore()
{
    if(NULL == xSemaphore)
    {
        xSemaphore = xSemaphoreCreateMutex();
    }

    while( xSemaphoreTake( xSemaphore, ( TickType_t ) 100 ) == 0 );
}

void uart_ctrl::releaseSemaphore()
{
    xSemaphoreGive( xSemaphore );
}




