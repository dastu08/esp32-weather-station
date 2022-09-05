idf_component_register(
    SRCS "al_bmp180.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES general pl_i2c log freertos
)
