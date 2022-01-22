cmake_minimum_required(VERSION 3.13.4)

# In-place use of library in this example
list(APPEND EXTRA_COMPONENT_DIRS " ${CMAKE_SOURCE_DIR}/..")

# Project
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(wifi_reconnect_example)

# NOTE these dependencies are for example only
#ExternalComponent_Get(double_reset https://github.com/mdvorak-iot/esp-double-reset.git 717c54d)
#ExternalComponent_Get(wps_config https://github.com/mdvorak-iot/esp-wps-config.git 6d8fe29)
