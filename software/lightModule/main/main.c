#include "connect.h"
#include "data.h"
#include "espServer.h"
#include "gpio.h"
#include "mqtt.h"
#include "sdkconfig.h"

static xQueueHandle gpio_evt_queue = NULL;
nvs_handle handler;

static void gpio_isr_handler(void *arg) {
  uint32_t gpio_num = (uint32_t)arg;
  gpio_isr_handler_remove(gpio_num);
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task(void *arg) {
  uint32_t io_num;
  for (;;) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      esp_mqtt_client_handle_t client = get_client();
      int new_state = !get_light_state();
      update_light_state(new_state);
      get_light_state_str();
      const char *msg = (new_state == 1) ? "On" : "Off";
      if (client != NULL) {
        esp_mqtt_client_publish(client, device_config.topic, msg, 0, 1, 0);
      }
      vTaskDelay(pdMS_TO_TICKS(150));
      gpio_isr_handler_add(0, gpio_isr_handler, (void *)0);
    }
  }
}

int is_config_complete() {
  nvs_handle my_handle;
  esp_err_t err;

  // 1. Open NVS in Read-Only mode
  err = nvs_open("storage", NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
    return 0; // If we can't open it, it's definitely not "complete"
  }

  size_t ssid_len = 0;
  size_t pass_len = 0;
  size_t broker_len = 0;
  size_t topic_len = 0;

  // 2. Check if the keys exist and get their lengths
  // nvs_get_str with NULL as the buffer returns the length including null
  // terminator
  esp_err_t e1 = nvs_get_str(my_handle, "ssid", NULL, &ssid_len);
  esp_err_t e2 = nvs_get_str(my_handle, "password", NULL, &pass_len);
  esp_err_t e3 = nvs_get_str(my_handle, "broker", NULL, &broker_len);
  esp_err_t e4 = nvs_get_str(my_handle, "topic", NULL, &topic_len);

  // 3. Close the handle
  nvs_close(my_handle);

  // 4. Validate results
  // We check if all keys exist (ESP_OK) AND they aren't just empty strings ("")
  // Note: ssid_len of 1 means it's just a null terminator (empty string)
  if (e1 == ESP_OK && ssid_len > 1 && e2 == ESP_OK && pass_len > 1 &&
      e3 == ESP_OK && broker_len > 1 && e4 == ESP_OK && topic_len > 1) {
    return 1; // All critical fields are filled
  }
  return 0;
}

void app_main(void) {
  // nvs_flash_erase();
  ESP_ERROR_CHECK(nvs_flash_init());

  ESP_ERROR_CHECK(esp_netif_init());

  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  initialize_gpio();
  gpio_install_isr_service(0);
  xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);
  gpio_isr_handler_add(0, gpio_isr_handler, (void *)0);

  if (is_config_complete()) {
    load_config_from_nvs();
    wifi_init_sta();
    mqtt_app_start();
  } else {
    wifi_init_softap();
    start_webserver();
  };
}
