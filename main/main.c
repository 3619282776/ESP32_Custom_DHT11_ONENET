#include <stdio.h>
#include "WIFI/wifi_driver.h"
#include "Console/console_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "DHT11/dht.h"
#include "DHT11/DHT11_driver.h"
#include "MQTT/mqtt_driver.h"
void app_main(void)
{
    nvs_flash_init();
    Wifi_sta_init("112233","123456789");
    console_init();
}
