//
//  ota_update.h
//  esp32-ota
//
//  Updating the firmware over the air.
//
//  Created by Andreas Schweizer on 25.11.2016.
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

#ifndef __OTA_UPDATE_H__
#define __OTA_UPDATE_H__

extern "C"{
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <esp_spi_flash.h>
#include <esp_partition.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
}

typedef enum {
    OTA_OK = 0,
    OTA_ERR_PARTITION_NOT_FOUND = 1,
    OTA_ERR_PARTITION_NOT_ACTIVATED = 2,
    OTA_ERR_BEGIN_FAILED = 3,
    OTA_ERR_WRITE_FAILED = 4,
    OTA_ERR_END_FAILED = 5,
} ota_update_result;

class ota_update{
public:
	ota_update();
	~ota_update();
	void init();
	int inProgress();
	ota_update_result begin();
	ota_update_result writeHexData(const char *hexData, size_t len);
	ota_update_result end();
	void dumpInformation();
	static const esp_partition_t* findNextBootPartition();
	static void dump128bytes(uint32_t addr, uint8_t *p);
	static void dump16bytes(uint32_t addr, uint8_t *p);
	static ota_update* getInstance();
};

#endif // __OTA_UPDATE_H__
