/*
 * Define pins for board
 * This is firmware for the error mushroom boards in the first order
 *
 * Created: May 09, 2016
 * Author: Tan Do
 * Company: AgrHub - Bee Team
 * Email: dmtan@agrhub.com
 * Website: agrhub.com
 */

#ifndef MAIN_h
#define MAIN_h

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SENSE_HYDRO_NAME         "ATSG M2M"
#define SENSE_HYDRO_CODE         "atsg_01"

//define configuration values
#define COMPANY_NAME "AGRHUB CO.,LTD."
#define PRODUCT_NAME "ATSG M2M"
#define PRODUCT_RELEASE_DATE 1588409905997

#define HARDWARE_VERSION "atsg_01"
#define SOFTWARE_VERSION "0.0.3"
#define SOFTWARE_DATE 1588409905997

#define AP_SSID "ATSG"
#define AP_PWD "12345678"

#define UPDATE_API "http://farmapi.agrhub.com/hardware/checkSW"

enum SWUpdateMode{
	SW_UPDATE_MODE_DEVELOPING,
	SW_UPDATE_MODE_ALPHA,
	SW_UPDATE_MODE_BETA,
	SW_UPDATE_MODE_STABLE
};

enum SWBootMode{
	SW_BOOT_MODE_RUN,
	SW_BOOT_MODE_OTA_UPDATE,
	SW_BOOT_MODE_OTA_INIT
};

struct sw_update_struct{
  char update_version[10] = "";
  SWUpdateMode update_state = SW_UPDATE_MODE_STABLE;
  char update_description[512] = "";
  char update_url[256] = "";
  bool is_active = false;
  uint64_t update_date = 0;
};

typedef void (*CallBackTypeVoid)();
typedef void (*CallBackTypeString)(const char*, const size_t);
typedef void (*CallBackTypeChar)(const uint8_t*, const size_t);
typedef void (*CallBackTypeUpdate)(sw_update_struct*);

#endif
