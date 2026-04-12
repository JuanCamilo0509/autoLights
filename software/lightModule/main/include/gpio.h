#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"

// GPIO pin for factory reset (erasing NVS memory)
// GPIO3 (RX) is used as the factory reset pin
#define FACTORY_RESET_GPIO GPIO_NUM_3

void initialize_gpio();

// GPIO for light control
void update_light_state(int new_state);
int get_light_state(void);
const char *get_light_state_str(void);

// Factory reset ISR
void initialize_factory_reset_gpio(void);
void IRAM_ATTR factory_reset_isr_handler(void *arg);
