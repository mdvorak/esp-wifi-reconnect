#ifndef WIFI_RECONNECT_H
#define WIFI_RECONNECT_H

#include <esp_err.h>

#ifdef __cplusplus
extern "C"
{
#endif

    const uint32_t WIFI_RECONNECT_CONNECT_TIMEOUT = 15000;

#ifdef __cplusplus
    esp_err_t wifi_reconnect_start(bool enable = true, uint32_t connect_timeout = WIFI_RECONNECT_CONNECT_TIMEOUT);
#else
esp_err_t wifi_reconnect_start(bool enable, uint32_t connect_timeout);
#endif

    void wifi_reconnect_enable(bool enable);

    inline void wifi_reconnect_pause()
    {
        wifi_reconnect_enable(false);
    }

    inline void wifi_reconnect_resume()
    {
        wifi_reconnect_enable(true);
    }

#ifdef __cplusplus
}
#endif

#endif
