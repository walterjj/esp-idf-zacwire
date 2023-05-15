set(req driver)

idf_component_register(
    SRCS "ZACwire.cpp"
    INCLUDE_DIRS "."
    REQUIRES ${req}
)