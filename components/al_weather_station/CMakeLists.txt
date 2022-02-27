idf_component_register(
    SRCS "al_weather_station.c"
    INCLUDE_DIRS "."
    REQUIRES esp_event esp_timer
    PRIV_REQUIRES general al_bmp180 al_crypto heartbeat pl_udp json
)
