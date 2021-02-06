#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <wifi_reconnect.h>
#include <nvs_flash.h>
#include <double_reset.h>
#include <wps_config.h>

static const char TAG[] = "example";

void setup()
{
	esp_log_level_set("wifi", ESP_LOG_WARN);
	esp_log_level_set("wifi_reconnect", ESP_LOG_DEBUG);

	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Check double reset
	// NOTE this should be called as soon as possible, ideally right after nvs init
	bool reconfigure = false;
	ESP_ERROR_CHECK(double_reset_start(&reconfigure, 5000));

	// Initalize WiFi
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());

	// Reconnection watch
	ESP_ERROR_CHECK(wifi_reconnect_start()); // NOTE this must be called before connect, otherwise it might miss connected event

	// Start WPS if WiFi is not configured, or reconfiguration was requested
	if (!wifi_reconnect_is_ssid_stored() || reconfigure)
	{
		ESP_LOGI(TAG, "reconfigure request detected, starting WPS");
		ESP_ERROR_CHECK(wps_config_start());
	}
	else
	{
		// Connect now
		ESP_ERROR_CHECK(esp_wifi_connect());
	}

	// Wait for WiFi
	ESP_LOGI(TAG, "waiting for wifi");
	if (!wifi_reconnect_wait_for_connection(AUTO_WPS_TIMEOUT_MS + WIFI_RECONNECT_CONNECT_TIMEOUT_MS))
	{
		ESP_LOGE(TAG, "failed to connect to wifi!");
		// NOTE either fallback into emergency operation mode, do nothing, restart..
	}

	// Setup complete
	ESP_LOGI(TAG, "started");
}

void loop()
{
	vTaskDelay(1);
}

extern "C" [[noreturn]] void app_main()
{
	setup();
	for (;;)
	{
		loop();
	}
}
