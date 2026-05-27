#include <stdio.h>
#include "WIFI/wifi_driver.h"
#include "Console/console_driver.h"
#include "nvs_flash.h"
void app_main(void)
{
    nvs_flash_init();
    Wifi_sta_init("112233","123456789");
    console_init();

}
