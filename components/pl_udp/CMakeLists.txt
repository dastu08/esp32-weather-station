idf_component_register(
    SRCS "pl_udp.c"
    INCLUDE_DIRS "."
    REQUIRES esp_event
    PRIV_REQUIRES general log esp_netif lwip freertos
)
