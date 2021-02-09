cmake_minimum_required(VERSION 3.11.0)
include(ExternalComponent.cmake)

# In-place use of library in this example
list(APPEND EXTRA_COMPONENT_DIRS " ${CMAKE_SOURCE_DIR}/..")

# NOTE these dependencies are for example only
ExternalComponent_Get(double_reset https://github.com/mdvorak-iot/esp-double-reset.git 4af4172)
ExternalComponent_Get(wps_config https://github.com/mdvorak-iot/esp-wps-config.git fbd45e3)

# Project
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(example)
