idf_component_register(
    SRCS "blink_test.c"
    INCLUDE_DIRS "."
        REQUIRES led_strip esp_wifi esp_event nvs_flash esp_http_server
)
