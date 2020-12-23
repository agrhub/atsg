#ifndef HANG_TIMER_INTERRUPT_H
#define HANG_TIMER_INTERRUPT_H

extern "C" {
#include <stdio.h>
#include "esp_types.h"
#include <esp_log.h>
#include <stddef.h> 
#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "driver/timer.h"
}

extern void IRAM_ATTR timer_prevent_hang_isr(void* arg);

static void init_prevent_hang_timer(int timer_period_us)
{
    ESP_LOGI("System", "init_prevent_hang_timer");
    timer_config_t config = {
            .alarm_en = true,
            .counter_en = false,
            .intr_type = TIMER_INTR_LEVEL,
            .counter_dir = TIMER_COUNT_UP,
            .auto_reload = true,
            .divider = 80   /* 1 us per tick */
    };
    
    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, timer_period_us);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_prevent_hang_isr, (void*)TIMER_0, ESP_INTR_FLAG_IRAM, NULL);
    timer_start(TIMER_GROUP_0, TIMER_0);
};

#endif