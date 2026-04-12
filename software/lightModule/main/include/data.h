#ifndef GLOBALS_H
#define GLOBALS_H

// Define the structure
typedef struct {
    char ssid[32];
    char password[64];
    char broker[64];
    char topic[64];
} config_data_t;

// Declare the variable as external
extern config_data_t device_config;

// Function prototype to sync RAM with NVS
void load_config_from_nvs(void);

#endif
