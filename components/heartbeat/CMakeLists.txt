idf_component_register(
    SRCS "heartbeat.c"
    INCLUDE_DIRS "."
    REQUIRES esp_timer esp_event
    PRIV_REQUIRES pl_udp
)
