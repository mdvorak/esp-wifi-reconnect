#include "wifi_reconnect.h"
#include <esp_log.h>
#include <esp_wifi.h>

static const char TAG[] = "wifi_reconnect";
static const int CONNECT_TIMEOUT = 15000;

static volatile bool reconnect_enabled = false;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static bool is_ssid_stored()
{
  wifi_config_t conf;
  ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &conf));
  return conf.sta.ssid[0] != '\0';
}

static bool is_wifi_connected()
{
  // TODO
  return true;
}

// NOTE it is intentionally done via background task, and not using events, since it is much more reliable
// NOTE also it is more reliable then autoReconnect functionality on ESP32
static void wifi_reconnect_task(void *)
{
  // Timer support
  const uint8_t DELAYS[] = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233};
  uint8_t failures = 0;

  ESP_LOGD(TAG, "Starting WiFi reconnect loop");

  // Infinite task loop
  for (;;)
  {
    if (reconnect_enabled && !is_wifi_connected() && is_ssid_stored())
    {
      // Simple back-off algorithm
      TickType_t waitFor = DELAYS[failures] * 1000;
      failures = MIN(sizeof(DELAYS) / sizeof(uint8_t) - 1, failures + 1);

      // Delay
      ESP_LOGI(TAG, "Wifi reconnect - waiting approx %d ms", waitFor);
      vTaskDelay(waitFor / portTICK_PERIOD_MS);

      // Start reconnect
      ESP_LOGI(TAG, "WiFi auto reconnecting");
      esp_wifi_connect();

      // Wait for connection
      vTaskDelay(CONNECT_TIMEOUT / portTICK_PERIOD_MS);

      // Reset failures
      if (is_wifi_connected())
      {
        failures = 0;
      }
    }
    else
    {
      // Simply wait for a moment
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

esp_err_t wifi_reconnect_init(bool enable)
{
  wifi_reconnect_enable(enable);
  auto ret = xTaskCreate(wifi_reconnect_task, "wifi_reconnect", 4096, nullptr, tskIDLE_PRIORITY + 1, nullptr);
  return ret == pdPASS ? ESP_OK : ESP_FAIL;
}

void wifi_reconnect_enable(bool enable)
{
  reconnect_enabled = enable;
}
