idf_component_register(
    SRCS "dl_wifi.c"
    INCLUDE_DIRS "."
    REQUIRES esp_event
    PRIV_REQUIRES general log esp_netif esp_wifi lwip
)
