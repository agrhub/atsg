/* Created 19 Nov 2016 by Chris Osborn <fozztexx@fozztexx.com>
 * http://insentricity.com
 *
 * Uses the RMT peripheral on the ESP32 for very accurate timing of
 * signals sent to the WS2812 LEDs.
 *
 * This code is placed in the public domain (or CC0 licensed, at your option).
 */

#include "rgb_led.h"
#include "config.h"
extern "C"
{
  #include <freertos/FreeRTOS.h>
  #include <freertos/semphr.h>
  #include <driver/gpio.h>
  #include <stdio.h>
  #include <stdlib.h>
}

void rgb_init()
{
  gpio_pad_select_gpio(RGB_RED_PIN);
  gpio_pad_select_gpio(RGB_GREEN_PIN);
  gpio_pad_select_gpio(RGB_BLUE_PIN);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(RGB_RED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(RGB_GREEN_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(RGB_BLUE_PIN, GPIO_MODE_OUTPUT);

  /* Set default LED state */
  rgb_config init_color = makeRGBVal(0x20, 0x0, 0x0);
  rgb_setColors(init_color);
}

void rgb_setColors(rgb_config array)
{
  uint8_t red = array.r;
  uint8_t green = array.g;
  uint8_t blue = array.b;
  if(red > 0)
  {
    gpio_set_level(RGB_RED_PIN, 0);
  }
  else
  {
    gpio_set_level(RGB_RED_PIN, 1); 
  }

  if(green > 0)
  {
    gpio_set_level(RGB_GREEN_PIN, 0); 
  }
  else
  {
    gpio_set_level(RGB_GREEN_PIN, 1); 
  }

  if(blue > 0)
  {
    gpio_set_level(RGB_BLUE_PIN, 0); 
  }
  else
  {
    gpio_set_level(RGB_BLUE_PIN, 1);  
  }
}
