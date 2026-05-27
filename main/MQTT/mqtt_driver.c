#include "mqtt_driver.h"
#include "stdlib.h"
#include "stdint.h"
#include "mqtt_client.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "MQTT/mqtt_driver.h"
#include "DHT11/DHT11_driver.h"
esp_mqtt_client_handle_t client = NULL;
TaskHandle_t* task_handle=NULL;
#define TOPIC "$sys/3D2J66rvKZ/ESP32/thing/property/post"
char* buffer="{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"tem\":{\"value\":55.5}}}";

static void mqtt_backcall_functon(void *arg, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    switch(event_id)
    {
        case MQTT_EVENT_CONNECTED:
            printf("MQTT connected\n");
            xTaskCreate(dht11_get, "dht_mqtt", 2048, NULL, 5,task_handle);
            // 订阅属性上报回复主题，用于接收平台处理结果
            esp_mqtt_client_subscribe(event->client, "$sys/3D2J66rvKZ/ESP32/thing/property/post/reply", 1);
            break;
        case MQTT_EVENT_DATA:
            // 打印平台返回的数据（例如上报成功或错误信息）
            printf("Received data: %.*s\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_DISCONNECTED:
            printf("MQTT disconnected and deletd task\n");
            vTaskDelete(*task_handle);
            break;
        case MQTT_EVENT_ERROR:
            printf("MQTT error\n");
            break;
        default:
            break;
    }
}

void mqtt_init()
{
    esp_mqtt_client_config_t t={
        .broker.address.uri=MQTT_BROKER,
        .credentials.client_id=MQTT_CLINET_ID,
        .credentials.username=MQTT_USERNAME,
        .credentials.authentication.password=MQTT_TOKEN,
    };
    client = esp_mqtt_client_init(&t);
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_backcall_functon, NULL);
    esp_mqtt_client_start(client);
}
void mqtt_send(float tem)
{
    char temp_str[128]={0};
    sprintf(temp_str,"{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"tem\":{\"value\":%.1f}}}",tem);
    esp_mqtt_client_publish(client,TOPIC, temp_str, 0, 1, 0);
    printf("mqtt send successfully\n");
}