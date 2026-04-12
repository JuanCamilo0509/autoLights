#include "data.h"
#include "nvs_flash.h"
#include "esp_log.h"

// Allocate the actual memory here
config_data_t device_config = {0}; 

void load_config_from_nvs(void) {
    nvs_handle handle;
    if (nvs_open("storage", NVS_READONLY, &handle) == ESP_OK) {
        size_t size;
        
        size = sizeof(device_config.ssid);
        nvs_get_str(handle, "ssid", device_config.ssid, &size);
        
        size = sizeof(device_config.password);
        nvs_get_str(handle, "password", device_config.password, &size);
        
        size = sizeof(device_config.broker);
        nvs_get_str(handle, "broker", device_config.broker, &size);
        
        size = sizeof(device_config.topic);
        nvs_get_str(handle, "topic", device_config.topic, &size);
        
        nvs_close(handle);
        ESP_LOGI("CONFIG", "Globals loaded from NVS");
    }
}
