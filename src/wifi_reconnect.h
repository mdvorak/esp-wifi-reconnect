#ifndef WIFI_RECONNECT_H
#define WIFI_RECONNECT_H

#include <esp_err.h>

esp_err_t wifi_reconnect_init(bool enable = true);

void wifi_reconnect_enable(bool enable);

inline void wifi_reconnect_pause()
{
    wifi_reconnect_enable(false);
}

inline void wifi_reconnect_resume()
{
    wifi_reconnect_enable(true);
}

#endif
