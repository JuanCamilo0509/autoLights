#include "connect.h"
#include "gpio.h"
#include "mqtt.h"
#include "sdkconfig.h"

static xQueueHandle gpio_evt_queue = NULL;

static void gpio_isr_handler(void *arg) {
  uint32_t gpio_num = (uint32_t)arg;
  gpio_isr_handler_remove(gpio_num);
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void *arg) {
  uint32_t io_num;
  for (;;) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      esp_mqtt_client_handle_t client = get_client();
      int new_state = !get_light_state();
      update_light_state(new_state);
      const char *msg = (new_state == 1) ? "On" : "Off";
      esp_mqtt_client_publish(client, CONFIG_TOPIC, msg, 0, 1, 0);
      vTaskDelay(pdMS_TO_TICKS(150));
      gpio_isr_handler_add(0, gpio_isr_handler, (void *)0);
    }
  }
}

void app_main(void) {
  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

  ESP_ERROR_CHECK(esp_netif_init());

  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  initialize_gpio();

  gpio_install_isr_service(0);
  xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);
  gpio_isr_handler_add(0, gpio_isr_handler, (void *)0);

  wifi_init_sta();

  mqtt_app_start();
}
