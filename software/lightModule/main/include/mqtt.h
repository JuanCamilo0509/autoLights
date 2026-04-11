#pragma once
static const char *TAG = "MQTTS";
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "gpio.h"
#include "nvs_flash.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_tls.h"
#include "mqtt_client.h"

void mqtt_app_start();
esp_mqtt_client_handle_t get_client();
