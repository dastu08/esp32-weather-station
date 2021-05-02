idf_component_register(
    SRCS "pl_i2c.c"
    INCLUDE_DIRS "."
    REQUIRES driver
    PRIV_REQUIRES general
)
