#include "connect.h"
#include "data.h"
#include "espServer.h"
#include "gpio.h"
#include "mqtt.h"
#include "sdkconfig.h"

static xQueueHandle gpio_evt_queue = NULL;

static void gpio_isr_handler(void *arg) {
  uint32_t gpio_num = (uint32_t)arg;
  gpio_isr_handler_remove(gpio_num);
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void light_toggle_task(void *arg) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    int new_state = !get_light_state();
    update_light_state(new_state);
    get_light_state_str();

    const char *msg = (new_state == 1) ? "On" : "Off";
    esp_mqtt_client_handle_t client = get_client();
    if (client != NULL) {
      esp_mqtt_client_publish(client, device_config.topic, msg, 0, 1, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(150));
    gpio_isr_handler_add(0, gpio_isr_handler, (void *)0);
  }
}

static void factory_reset_task(void *arg) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    nvs_flash_erase();
    esp_restart();
  }
}

static TaskHandle_t light_task_handle = NULL;
static TaskHandle_t factory_task_handle = NULL;

static void gpios_tasks(void *arg) {
  uint32_t io_num;
  for (;;) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      if (io_num == 0) {
        if (light_task_handle != NULL) {
          xTaskNotifyGive(light_task_handle);
        }
      } else if (io_num == 5) {
        if (factory_task_handle != NULL) {
          xTaskNotifyGive(factory_task_handle);
        }
      }
    }
  }
};

int is_config_complete() {
  nvs_handle my_handle;
  esp_err_t err;

  err = nvs_open("storage", NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
    return 0;
  }

  size_t ssid_len = 0;
  size_t pass_len = 0;
  size_t broker_len = 0;
  size_t topic_len = 0;

  esp_err_t e1 = nvs_get_str(my_handle, "ssid", NULL, &ssid_len);
  esp_err_t e2 = nvs_get_str(my_handle, "password", NULL, &pass_len);
  esp_err_t e3 = nvs_get_str(my_handle, "broker", NULL, &broker_len);
  esp_err_t e4 = nvs_get_str(my_handle, "topic", NULL, &topic_len);

  nvs_close(my_handle);

  if (e1 == ESP_OK && ssid_len > 1 && e2 == ESP_OK && pass_len > 1 &&
      e3 == ESP_OK && broker_len > 1 && e4 == ESP_OK && topic_len > 1) {
    return 1;
  }
  return 0;
}

void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());

  ESP_ERROR_CHECK(esp_netif_init());

  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  initialize_gpio();
  xTaskCreate(light_toggle_task, "light_task", 2048, NULL, 9,
              &light_task_handle);
  xTaskCreate(factory_reset_task, "factory_task", 2048, NULL, 8,
              &factory_task_handle);
  xTaskCreate(gpios_tasks, "gpio_task", 2048, NULL, 10, NULL);
  gpio_isr_handler_add(0, gpio_isr_handler, (void *)0);
  gpio_isr_handler_add(5, gpio_isr_handler, (void *)5);

  if (is_config_complete()) {
    load_config_from_nvs();
    wifi_init_sta();
    mqtt_app_start();
  } else {
    wifi_init_softap();
    start_webserver();
  };
}
