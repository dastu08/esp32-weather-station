cmake_minimum_required(VERSION 3.5)

# limit the list of used components
set(COMPONENTS esptool_py main general dl_wifi pl_udp pl_i2c al_bmp180 heartbeat al_weather_station al_crypto)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# name the project
project(esp32-weather-station)

# set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components/esp_http_server)

