idf_component_register(SRCS "main.c" "WIFI/wifi_driver.c" "Console/console_driver.c" "MQTT/mqtt_driver.c"
                    REQUIRES nvs_flash esp_netif esp_wifi console driver mqtt
                    INCLUDE_DIRS ".")
