idf_component_register(
    SRCS "al_weather_station.c"
    INCLUDE_DIRS "."
    REQUIRES esp_event
    PRIV_REQUIRES general json
)
