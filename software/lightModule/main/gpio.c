#include "gpio.h"
#include "esp_system.h"
#include "nvs_flash.h"

int light_state = 0;

void initialize_gpio() {
  gpio_config_t io_confI = {};
  io_confI.intr_type = GPIO_INTR_NEGEDGE;
  io_confI.mode = GPIO_MODE_INPUT;
  io_confI.pull_down_en = 0;
  io_confI.pull_up_en = 1;
  io_confI.pin_bit_mask = (1ULL << 0);
  gpio_config(&io_confI);

  io_confI.intr_type = GPIO_INTR_DISABLE;
  io_confI.mode = GPIO_MODE_OUTPUT;
  io_confI.pull_down_en = 0;
  io_confI.pull_up_en = 0;
  io_confI.pin_bit_mask = (1ULL << 2);

  gpio_config(&io_confI);
}

void initialize_factory_reset_gpio(void) {
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 1;
  io_conf.pin_bit_mask = (1ULL << FACTORY_RESET_GPIO);
  gpio_config(&io_conf);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(FACTORY_RESET_GPIO, factory_reset_isr_handler, NULL);
  ESP_LOGI("FACTORY_RESET", "Factory reset ISR initialized on GPIO%u", FACTORY_RESET_GPIO);
}

void IRAM_ATTR factory_reset_isr_handler(void *arg) {
  // Disable further interrupts to prevent bouncing
  gpio_isr_handler_remove(FACTORY_RESET_GPIO);

  // Schedule a software restart with delay to allow ISR to complete
  // Using esp_restart in ISR is safe for simple reset operations
  esp_restart();
}

void update_light_state(int new_state) {
  light_state = new_state;
  gpio_set_level(2, !light_state);
}

int get_light_state(void) { return light_state; }

const char *get_light_state_str(void) {
  return (light_state == 1) ? "On" : "Off";
}
