#include <esp_wifi.h>
#include <esp_log.h>
#include <wifi_reconnect.h>

void setup()
{
	// Setup complete
	ESP_LOGI("example", "started");
}

void loop()
{
	vTaskDelay(1);
}

extern "C"
{
	void app_main()
	{
		setup();
		for (;;)
		{
			loop();
		}
	}
}