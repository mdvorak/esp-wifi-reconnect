#include "wifi_reconnect.h"
#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/event_groups.h>

static const char TAG[] = "wifi_reconnect";

// Reconnect incremental backoff, in seconds
static const uint8_t DELAYS[] = {0, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233};
static const int RECONNECT_BIT = BIT0;
static const int CONNECTED_BIT = BIT1;

static EventGroupHandle_t wifi_event_group;
static uint32_t connect_timeout = WIFI_RECONNECT_CONNECT_TIMEOUT_MS;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static inline bool is_ssid_stored(wifi_config_t &conf)
{
  esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &conf);
  return err == ESP_OK && conf.sta.ssid[0] != '\0';
}

static bool should_reconnect()
{
  EventBits_t bits = xEventGroupGetBits(wifi_event_group);
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
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));

    // Mark as connected, also enable reconnect automatically
    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT | RECONNECT_BIT);
  }
}

// NOTE it is intentionally done via background task, and not using events, since it is more reliable
_Noreturn static void wifi_reconnect_task(void *)
{
  uint8_t failures = 0;

  ESP_LOGI(TAG, "reconnect loop started, connect timeout %d ms", connect_timeout);

  // Wait before looping
  xEventGroupWaitBits(wifi_event_group, RECONNECT_BIT, pdFALSE, pdTRUE, connect_timeout / portTICK_PERIOD_MS);

  // Infinite task loop
  for (;;)
  {
    wifi_config_t conf = {};
    if (should_reconnect() && is_ssid_stored(conf))
    {
      // Simple back-off algorithm
      TickType_t waitFor = DELAYS[failures] * 1000;
      failures = MIN(sizeof(DELAYS) / sizeof(uint8_t) - 1, failures + 1);

      // Delay
      if (waitFor > 0)
      {
        ESP_LOGI(TAG, "waiting for %d ms", waitFor);
        vTaskDelay(waitFor / portTICK_PERIOD_MS);
      }

      // Start reconnect
      ESP_LOGI(TAG, "connecting to '%s'", conf.sta.ssid);
      ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());

      // Wait for connection
      bool connected = (xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, pdFALSE, pdTRUE, connect_timeout / portTICK_PERIOD_MS) & CONNECTED_BIT) != 0;

      // Reset failures, if connected successfully
      if (connected)
      {
        failures = 0;
        ESP_LOGI(TAG, "connected successfully");
      }
      else
      {
        ESP_LOGI(TAG, "connect timeout");
      }
    }
    else
    {
      // Simply wait for a moment
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

esp_err_t wifi_reconnect_start()
{
  esp_err_t err;

  // Prepare event group
  wifi_event_group = xEventGroupCreate();
  configASSERT(wifi_event_group);

  // Register event handlers
  err = esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL);
  if (err != ESP_OK)
    return err;

  err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);
  if (err != ESP_OK)
    return err;

  // Create background task
  auto ret = xTaskCreate(wifi_reconnect_task, "wifi_reconnect", 4096, NULL, tskIDLE_PRIORITY + 1, NULL);
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

bool wifi_reconnect_is_ssid_stored()
{
  wifi_config_t conf;
  return is_ssid_stored(conf);
}

bool wifi_reconnect_is_connected()
{
  EventBits_t bits = xEventGroupGetBits(wifi_event_group);
  return (bits & CONNECTED_BIT) != 0;
}

bool wifi_reconnect_wait_for_connection(uint32_t timeout_ms)
{
  EventBits_t bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, pdFALSE, pdTRUE, timeout_ms / portTICK_PERIOD_MS);
  return (bits & CONNECTED_BIT) != 0;
}
