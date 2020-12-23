#ifndef __UART_CONTROL_h__
#define __UART_CONTROL_h__
#include "config.h"

class uart_ctrl
{
public:
	uart_ctrl();
	~uart_ctrl();
	static uart_ctrl* getInstance();
	static void rx_task(void* param);
	void init();
	void setRxCallback(CallBackTypeChar cb);
	bool sendData(const char* data, const bool needResponse = true);
	void setData(const uint8_t* data, const size_t length = 0);
	uint8_t* readData(uint16_t &len);
	bool isBusy;
	bool isWaitingData;
	static const int RX_BUF_SIZE = 1024;
	char mLastCMD[128];
private:
	static void createAndTakeSemaphore();
    static void releaseSemaphore();

    static uart_ctrl* s_pInstance;
    uint8_t mData[RX_BUF_SIZE];
    uint16_t mLength = 0;
    static SemaphoreHandle_t xSemaphore;

    static const char mTag[];
};

#endif //__UART_CONTROL_h__