#include "wifi_reconnect.h"
#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/event_groups.h>

static const char TAG[] = "wifi_reconnect";

// Reconnect exponential backoff, in seconds
static const uint8_t DELAYS[] = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233};
static const int RECONNECT_BIT = BIT0;
static const int CONNECTED_BIT = BIT1;

static EventGroupHandle_t wifi_event_group;
static uint32_t connect_timeout = WIFI_RECONNECT_CONNECT_TIMEOUT;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static bool is_ssid_stored()
{
  wifi_config_t conf;
  esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &conf);
  return err == ESP_OK && conf.sta.ssid[0] != '\0';
}

static bool should_reconnect()
{
  auto bits = xEventGroupGetBits(wifi_event_group);
  return (bits & RECONNECT_BIT) != 0 && (bits & CONNECTED_BIT) == 0;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    ESP_LOGI(TAG, "disconnected");
    xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    auto *event = static_cast<ip_event_got_ip_t *>(event_data);
    ESP_LOGI(TAG, "got IP: " IPSTR, IP2STR(&event->ip_info.ip));

    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
  }
}

// NOTE it is intentionally done via background task, and not using events, since it is more reliable
static void wifi_reconnect_task(void *)
{
  uint8_t failures = 0;

  ESP_LOGI(TAG, "reconnect loop started, connection timeout %d ms", connect_timeout);

  // Infinite task loop
  for (;;)
  {
    if (should_reconnect() && is_ssid_stored())
    {
      // Simple back-off algorithm
      TickType_t waitFor = DELAYS[failures] * 1000;
      failures = MIN(sizeof(DELAYS) / sizeof(uint8_t) - 1, failures + 1);

      // Delay
      ESP_LOGI(TAG, "waiting approx %d ms", waitFor);
      vTaskDelay(waitFor / portTICK_PERIOD_MS);

      // Start reconnect
      ESP_LOGI(TAG, "auto reconnecting");
      esp_wifi_connect();

      // Wait for connection
      // NOTE here is the rare race-condition, if esp_wifi_connect() will succeed before calling wait, it will never pick up state change and will timeout - which does not do any harm in the end
      bool connected = (xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, pdFALSE, pdTRUE, connect_timeout / portTICK_PERIOD_MS) & CONNECTED_BIT) != 0;

      // Reset failures, if connected successfully
      if (connected)
      {
        ESP_LOGI(TAG, "connected successfully");
        failures = 0;
      }
      else
      {
        ESP_LOGI(TAG, "connection timeout");
      }
    }
    else
    {
      // Simply wait for a moment
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

esp_err_t wifi_reconnect_start(bool enable, uint32_t connect_timeout)
{
  // Prepare event group
  wifi_event_group = xEventGroupCreate();

  // Register event handlers
  esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

  // Set enabled flag
  wifi_reconnect_enable(enable);
  ::connect_timeout = connect_timeout;

  // Create background task
  auto ret = xTaskCreate(wifi_reconnect_task, "wifi_reconnect", 4096, nullptr, tskIDLE_PRIORITY + 1, nullptr);
  return ret == pdPASS ? ESP_OK : ESP_FAIL;
}

void wifi_reconnect_enable(bool enable)
{
  if (enable)
  {
    xEventGroupSetBits(wifi_event_group, RECONNECT_BIT);
    ESP_LOGI(TAG, "reconnect enabled");
  }
  else
  {
    xEventGroupClearBits(wifi_event_group, RECONNECT_BIT);
    ESP_LOGI(TAG, "reconnect disabled");
  }
}
