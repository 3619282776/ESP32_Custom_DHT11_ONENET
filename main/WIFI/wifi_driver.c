#include "wifi_driver.h"
#include "string.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "lwip/ip4_addr.h"
#include "freertos/queue.h"
#include "MQTT/mqtt_driver.h"
#include "esp_timer.h"
static esp_timer_handle_t timer_handle = NULL;
static int reconnect_attempts = 0;
#define MAX_RECONNECT_ATTEMPTS 5
#define RECONNECT_INTERVAL_MS 3000

static void reconnect_timer_cb(void *arg)
{
    if(reconnect_attempts > MAX_RECONNECT_ATTEMPTS )
    {
        printf("reconnect has five time stop reconnect\n");
        esp_timer_stop(timer_handle);
        reconnect_attempts=0;
    }
    else{
    reconnect_attempts++;
    printf("NOW is reconnect %d time\n",reconnect_attempts);
    esp_wifi_connect();
    }
}





static void init_reconnect_timer()//init esp_timer
{
    const esp_timer_create_args_t timer_args={
        .callback = &reconnect_timer_cb,
        .name ="wifi_reconnect_timer"
    };
    esp_timer_create(&timer_args,&timer_handle);
}


void wifi_event_cb(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data)
{
    if (event_base == WIFI_EVENT)//Wifi event 
    {
    switch(event_id)
    {
        case WIFI_EVENT_STA_START:
        {
            printf("STA start\n");
            esp_wifi_connect();
            break;
        }
        case WIFI_EVENT_STA_CONNECTED:
        {
            printf("Connect to AP\n");

            break;
        }
        case WIFI_EVENT_STA_DISCONNECTED:
        {
            printf("wifi has been disconnected\n");

            if (timer_handle && !esp_timer_is_active(timer_handle)) {
             esp_timer_start_periodic(timer_handle, RECONNECT_INTERVAL_MS * 1000);
                     }
            break;
        }
        case WIFI_EVENT_AP_STACONNECTED:
        {
            printf("Connect to AP\n");
            break;
        }
        case WIFI_EVENT_AP_START:
        {
            printf("AP start\n");
            break;
        }
        default:
        {
            break;
        }

    }
    }
    else if (event_base==IP_EVENT)//IP event
    {
        switch(event_id)
        { 
        
        case IP_EVENT_STA_GOT_IP:
        {
            esp_netif_ip_info_t *t = (esp_netif_ip_info_t*)event_data;
            printf("IP is " IPSTR "\n", IP2STR(&t->ip));
            if(timer_handle) esp_timer_stop(timer_handle);
            reconnect_attempts=0;
            mqtt_init();
            break;
        }
        case IP_EVENT_AP_STAIPASSIGNED:
        {
        ip_event_ap_staipassigned_t *event = (ip_event_ap_staipassigned_t *)event_data;
        esp_ip4_addr_t *ip = &event->ip; 
        char buffer[16];  // 足够存放 "192.168.10.2" 格式的IP
        sprintf(buffer, IPSTR, IP2STR(ip));   // IPSTR 宏展开为 "%d.%d.%d.%d"
        printf("IP: %s\n", buffer);           // 串口输出 IP
        break;
        }
    }
    }
}
static void wifi_init()
{
    esp_netif_init();//初始化TCP IP协议栈
    esp_event_loop_create_default();//创建默认事件循环
    esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event_cb,NULL);
    esp_event_handler_register(IP_EVENT,ESP_EVENT_ANY_ID,&wifi_event_cb,NULL);//注册默认事件的监听事件
    init_reconnect_timer();
}
void Wifi_sta_init(const char *ssid,const char *password)
{
    wifi_init();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t conf=WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&conf);
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t sta_conf={0};
    strlcpy((char *)sta_conf.sta.ssid, ssid, sizeof(sta_conf.sta.ssid));
    strlcpy((char *)sta_conf.sta.password, password, sizeof(sta_conf.sta.password));
    
    esp_wifi_set_config(WIFI_IF_STA,&sta_conf);
    esp_wifi_start();
}
void Wifi_ap_init(const char *ssid,const char *password)
{
    wifi_init();

    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 10, 1);      // 改成你想用的 IP
    IP4_ADDR(&ip_info.gw, 192, 168, 10, 1);      // 网关
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);// 子网掩码
    esp_netif_dhcps_stop(ap_netif);              // 先停掉默认 DHCP
    esp_netif_set_ip_info(ap_netif, &ip_info);   // 应用静态 IP
    esp_netif_dhcps_start(ap_netif);             // 重新启动 DHCP

    wifi_init_config_t cfg =WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_AP);

    wifi_config_t ap_cfg = {
        .ap = {
            .ssid_len = strlen(ssid),
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };
    strlcpy((char *)ap_cfg.ap.ssid, ssid, sizeof(ap_cfg.ap.ssid));
    strlcpy((char *)ap_cfg.ap.password, password, sizeof(ap_cfg.ap.password));
    esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);

    esp_wifi_start();
}
void wifi_apsta_init(const char *sta_ssid, const char *sta_password,const char *ap_ssid, const char *ap_password)
{
    wifi_init();

    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_APSTA);

    // STA 配置
    wifi_config_t sta_cfg = {0};
    strlcpy((char *)sta_cfg.sta.ssid, sta_ssid, sizeof(sta_cfg.sta.ssid));
    strlcpy((char *)sta_cfg.sta.password, sta_password, sizeof(sta_cfg.sta.password));

    esp_wifi_set_config(WIFI_IF_STA, &sta_cfg);

    // AP 配置
    wifi_config_t ap_cfg = {
        .ap = {
            .ssid_len = strlen(ap_ssid),
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };
    strlcpy((char *)ap_cfg.ap.ssid, ap_ssid, sizeof(ap_cfg.ap.ssid));
    strlcpy((char *)ap_cfg.ap.password, ap_password, sizeof(ap_cfg.ap.password));
    
    esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);

    esp_wifi_start();
}