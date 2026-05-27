#ifndef __WIFI_DRIVER__
#define __WIFI_DRIVER__
void Wifi_sta_init(const char *ssid,const char *password);
void Wifi_ap_init(const char *ssid,const char *password);
void wifi_apsta_init(const char *sta_ssid, const char *sta_password,const char *ap_ssid, const char *ap_password);
#endif