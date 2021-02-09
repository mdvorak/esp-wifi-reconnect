cmake_minimum_required(VERSION 3.11.0)

idf_component_register(
    SRC_DIRS src
    INCLUDE_DIRS src
    REQUIRES freertos log esp_common esp_wifi
)
