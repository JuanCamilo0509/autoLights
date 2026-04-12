#include "gpio.h"

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

  io_confI.intr_type = GPIO_INTR_NEGEDGE;
  io_confI.mode = GPIO_MODE_INPUT;
  io_confI.pull_down_en = 0;
  io_confI.pull_up_en = 1;
  io_confI.pin_bit_mask = (1ULL << 5);
  gpio_config(&io_confI);

  gpio_install_isr_service(0);
}

void update_light_state(int new_state) {
  light_state = new_state;
  gpio_set_level(2, !light_state);
}

int get_light_state(void) { return light_state; }

const char *get_light_state_str(void) {
  return (light_state == 1) ? "On" : "Off";
}
