#ifndef __MQTT_H_
#define __MQTT_H_
#define MQTT_BROKER "mqtt://heclouds.com:1883"
#define MQTT_CLINET_ID "ESP32"
#define MQTT_USERNAME "3D2J66rvKZ"
#include "mqtt_client.h"
extern esp_mqtt_client_handle_t client;
#define MQTT_TOKEN "version=2018-10-31&res=products%2F3D2J66rvKZ%2Fdevices%2FESP32&et=2220846104&method=md5&sign=K1l6NUkyFDH08G8OLqYZYw%3D%3D"
void mqtt_init();
void mqtt_send(float tem);
#endif