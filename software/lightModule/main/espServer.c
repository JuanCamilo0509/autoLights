#include "espServer.h"
#include <ctype.h>

struct {
  char SSID[32];
  char PASSWORD[64];
  char BROKER[64];
  char TOPIC[64];
} data;

nvs_handle handler;

static const char *TAG = "APP";

static char *success_page() {
  return "<!DOCTYPE html>\
<html lang=\"en\">\
<head>\
  <meta charset=\"UTF-8\">\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
  <title>Success - Esp32</title>\
</head>\
<body style=\"display: flex; height: 100vh; margin: 0; flex-direction: column; align-items: center; justify-content: center; background: radial-gradient(circle, #1a1a1a, #000); color: #eee; font-family: \'Segoe UI\', sans-serif; text-align: center; gap: 20px;\">\
    <svg width=\"80\" height=\"80\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#2ecc71\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">\
        <path d=\"M22 11.08V12a10 10 0 1 1-5.93-9.14\"></path>\
        <polyline points=\"22 4 12 14.01 9 11.01\"></polyline>\
    </svg>\
    <div>\
        <h1 style=\"margin: 0; letter-spacing: 1px; color: #2ecc71;\">Success!</h1>\
        <p style=\"margin: 10px 0 0; font-size: 1.1rem; color: #aaa; max-width: 300px; line-height: 1.5;\">\
            Your ESP8266 has been configured and should be working.\
        </p>\
    </div>\
    <small style=\"color: #555; font-size: 0.75rem;\">\
        The device may reboot automatically.\
    </small>\
</body>\
</html>\
";
};
static char *formPage() {
  return "<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\">\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    <title>EspSetVariables</title>\
  </head>\
  <body style=\"display: flex; min-height: 100vh; margin: 0; flex-direction: column; align-items: center; justify-content: center; background: radial-gradient(circle, #1a1a1a, #000); color: #eee; font-family: \'Segoe UI\', sans-serif; gap: 12px;\">\
    <h1 style=\"margin: 0; letter-spacing: 1px; color: #4dabf7;\">Automatic Lights</h1>\
    <h2 style=\"margin: 0; font-weight: 400; font-size: 1.1rem; color: #aaa;\">Configuration</h2>\
    <button onclick=\"window.location.href=\'/test\'\" style=\"width: 262px; margin-top: 10px; padding: 12px; cursor: pointer; border-radius: 6px; border: none; background: #4dabf7; color: #fff; font-weight: bold; text-transform: uppercase; box-shadow: 0 4px 6px rgba(0,0,0,0.4);\">\
      Test\
    </button>\
    <small style=\"color: #777; margin-bottom: 15px; font-size: 0.8rem;\">Use this button to identify the light.</small>\
    <form action=\"./save.html\" method=\"GET\" style=\"display: flex; flex-direction: column; align-items: center; gap: 12px;\">\
      <input type=\"text\" name=\"ssid\" placeholder=\"SSID\" style=\"width: 240px; padding: 10px; border-radius: 6px; border: 1px solid #444; background: #222; color: #fff; outline: none;\">\
      <input type=\"password\" name=\"password\" placeholder=\"Password\" style=\"width: 240px; padding: 10px; border-radius: 6px; border: 1px solid #444; background: #222; color: #fff; outline: none;\">\
      <input type=\"text\" name=\"broker\" placeholder=\"Broker URL\" style=\"width: 240px; padding: 10px; border-radius: 6px; border: 1px solid #444; background: #222; color: #fff; outline: none;\">\
      <input type=\"text\" name=\"topic\" placeholder=\"Topic\" style=\"width: 240px; padding: 10px; border-radius: 6px; border: 1px solid #444; background: #222; color: #fff; outline: none;\">\
      <button type=\"submit\" style=\"width: 262px; margin-top: 10px; padding: 12px; cursor: pointer; border-radius: 6px; border: none; background: #2ecc71; color: #fff; font-weight: bold; text-transform: uppercase; box-shadow: 0 4px 6px rgba(0,0,0,0.4);\">\
        Save & Connect\
      </button>\
      <small style=\"color: #777; margin-bottom: 15px; font-size: 0.8rem;\"> After sending this information you\'ll be disconnect</small>\
    </form>\
  </body>\
</html>\
";
};

void url_decode(char *dst, const char *src) {
  char a, b;
  while (*src) {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

static esp_err_t get_handler_save(httpd_req_t *req) {
  size_t buf_len = httpd_req_get_url_query_len(req) + 1;

  if (buf_len <= 1) {
    ESP_LOGE(TAG, "Empty query string");
    return ESP_FAIL;
  }

  char *buf = malloc(buf_len);
  if (buf == NULL) {
    ESP_LOGE(TAG, "Failed to allocate query buffer");
    return ESP_FAIL;
  }

  if (httpd_req_get_url_query_str(req, buf, buf_len) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get query string");
    free(buf);
    return ESP_FAIL;
  }

  // Parse raw values
  httpd_query_key_value(buf, "ssid", data.SSID, sizeof(data.SSID));
  httpd_query_key_value(buf, "password", data.PASSWORD, sizeof(data.PASSWORD));
  httpd_query_key_value(buf, "broker", data.BROKER, sizeof(data.BROKER));
  httpd_query_key_value(buf, "topic", data.TOPIC, sizeof(data.TOPIC));
  free(buf);

  // URL decode
  char decoded_ssid[64];
  char decoded_pass[64];
  char decoded_broker[128];
  char decoded_topic[128];

  url_decode(decoded_ssid, data.SSID);
  url_decode(decoded_pass, data.PASSWORD);
  url_decode(decoded_broker, data.BROKER);
  url_decode(decoded_topic, data.TOPIC);

  ESP_LOGI(TAG, "Saving → SSID=%s BROKER=%s TOPIC=%s", decoded_ssid,
           decoded_broker, decoded_topic);

  nvs_handle handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return ESP_FAIL;
  }

  bool write_ok = (nvs_set_str(handle, "ssid", decoded_ssid) == ESP_OK) &&
                  (nvs_set_str(handle, "password", decoded_pass) == ESP_OK) &&
                  (nvs_set_str(handle, "broker", decoded_broker) == ESP_OK) &&
                  (nvs_set_str(handle, "topic", decoded_topic) == ESP_OK);

  if (!write_ok) {
    ESP_LOGE(TAG, "One or more nvs_set_str failed");
    nvs_close(handle);
    return ESP_FAIL;
  }

  err = nvs_commit(handle); // ← flush to flash
  nvs_close(handle);        // ← always close

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Config saved successfully, rebooting in 2s...");
  // ─────────────────────────────────────────────────────

  char *response = success_page();
  httpd_resp_send(req, response, strlen(response));

  vTaskDelay(pdMS_TO_TICKS(2000));
  esp_restart();
  return ESP_OK;
}
static esp_err_t get_handler_form(httpd_req_t *req) {
  char *response_message = formPage();
  httpd_resp_send(req, response_message, strlen(response_message));
  return ESP_OK;
};

httpd_uri_t form = {.uri = "/",
                    .method = HTTP_GET,
                    .handler = get_handler_form,
                    .user_ctx = NULL};

httpd_uri_t save = {.uri = "/save.html",
                    .method = HTTP_GET,
                    .handler = get_handler_save,
                    .user_ctx = NULL};

httpd_handle_t start_webserver(void) {
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  // Start the httpd server
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
    // Set URI handlers
    httpd_register_uri_handler(server, &form);
    httpd_register_uri_handler(server, &save);
    return server;
  }

  ESP_LOGI(TAG, "Error starting server!");
  return NULL;
}
