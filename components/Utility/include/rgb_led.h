/* Created 19 Nov 2016 by Chris Osborn <fozztexx@fozztexx.com>
 * http://insentricity.com
 *
 * This is a driver for the WS2812 RGB LEDs using the RMT peripheral on the ESP32.
 *
 * This code is placed in the public domain (or CC0 licensed, at your option).
 */

#ifndef RGB_DRIVER_H
#define RGB_DRIVER_H

#include <stdint.h>
extern "C" {
#include "ws2812.h"
}

extern void rgb_init();
extern void rgb_setColors(rgb_config array);

#endif /* RGB_DRIVER_H */
