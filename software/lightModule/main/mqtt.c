/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
   */

#include "mqtt.h"

static esp_mqtt_client_handle_t client = NULL;

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
  esp_mqtt_client_handle_t client = event->client;
  switch (event->event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    esp_mqtt_client_subscribe(client, CONFIG_TOPIC, 0);
    esp_mqtt_client_publish(client, CONFIG_TOPIC, get_light_state_str(), 0, 1,
                            0);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    if (strncmp(event->data, "Off", event->data_len) == 0) {
      update_light_state(0);
      ESP_LOGI(TAG, "!Turn OFF the LED!");
    } else if (strncmp(event->data, "On", event->data_len) == 0) {
      ESP_LOGI(TAG, "!Turn ON the LED!");
      update_light_state(1);
    }
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
  return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base,
           event_id);
  mqtt_event_handler_cb(event_data);
}

esp_mqtt_client_handle_t get_client() { return client; };

void mqtt_app_start() {
  esp_mqtt_client_config_t mqtt_cfg = {.uri = "mqtt://192.168.1.7:1883",
                                       .disable_auto_reconnect = false,
                                       .reconnect_timeout_ms = 500};
  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler,
                                 client);
  esp_mqtt_client_start(client);
}
