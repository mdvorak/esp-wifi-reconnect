cmake_minimum_required(VERSION 3.11.0)

idf_component_register(
        SRCS src/wifi_reconnect.c
        INCLUDE_DIRS include
        REQUIRES freertos log esp_common esp_wifi
)
