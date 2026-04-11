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

void initialize_gpio();

void update_light_state(int new_state);

int get_light_state(void);
const char *get_light_state_str(void);
