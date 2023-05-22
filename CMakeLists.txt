set(req driver esp_timer freertos)

idf_component_register(
    SRCS "ZACwire.cpp"
    INCLUDE_DIRS "."
    REQUIRES ${req}
)