#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <double_reset.h>
#include <wps_config.h>
#include <wifi_reconnect.h>

static const char TAG[] = "example";

static void setup()
{
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
	ESP_ERROR_CHECK(double_reset_start(&reconfigure, DOUBLE_RESET_DEFAULT_TIMEOUT));

	// Events
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Initalize WiFi
	ESP_ERROR_CHECK(esp_netif_init());
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg)); 
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());

	// Reconnection watch
	ESP_ERROR_CHECK(wifi_reconnect_start()); // NOTE this must be called before esp_wifi_connect, otherwise it might miss connected event

	// Start WPS if WiFi is not configured, or reconfiguration was requested
	if (!wifi_reconnect_is_ssid_stored() || reconfigure)
	{
		ESP_LOGI(TAG, "reconfigure request detected, waiting for wps");
		ESP_ERROR_CHECK(wps_config_start());
		// Wait for WPS to finish
		wifi_reconnect_wait_for_connection(WPS_CONFIG_TIMEOUT_MS);
	}

	// Connect now (needs to be called after WPS)
	wifi_reconnect_resume();

	// Wait for WiFi
	ESP_LOGI(TAG, "waiting for wifi");
	if (!wifi_reconnect_wait_for_connection(WIFI_RECONNECT_CONNECT_TIMEOUT_MS))
	{
		ESP_LOGE(TAG, "failed to connect to wifi!");
		// NOTE either fallback into emergency operation mode, do nothing, restart..
	}

	// Setup complete
	ESP_LOGI(TAG, "started");
}

static void run()
{
}

extern "C" void app_main()
{
	setup();
	run();
}
