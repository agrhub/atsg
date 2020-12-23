//
//  network.h
//  esp32-ota
//
//  Networking code for OTA demo application
//
//  Created by Andreas Schweizer on 02.12.2016.
//  Copyright © 2016 Classy Code GmbH
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef __NETWORK_H__
#define __NETWORK_H__
extern "C"{
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
}

class Socketota_update{
public:
	Socketota_update();
	~Socketota_update();
	void init();
	void openSocket(const char *ssid, const char *password);
	int isConnected();
	static Socketota_update* getInstance();
};


#endif // __NETWORK_H__
