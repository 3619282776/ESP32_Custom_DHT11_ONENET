#include "DHT11_driver.h"
#include "dht.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MQTT/mqtt_driver.h"
#define DHT11_SENSOR  DHT_TYPE_DHT11
void dht11_get(void *arg)
{
    float temperature, humidity;
    while(1)
    {
    dht_read_float_data(DHT11_SENSOR, DHT11_PIN, &temperature, &humidity);
    mqtt_send(temperature);
    vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
