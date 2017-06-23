#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "debug.h"
// Required for LIGHT_SLEEP_T delay mode
extern "C" {
#include "user_interface.h"
}

void initSleep() {
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
}

void endSleep() {
  wifi_fpm_close();
  wifi_set_opmode(STATION_MODE);
  wifi_station_connect();
  delay(100);
  dp("Woke up from sleep");
}

void enterSleep(unsigned long sleep_time_in_ms){
  wifi_set_opmode(NULL_MODE); 
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T); 
  wifi_fpm_open(); 
  wifi_fpm_do_sleep(sleep_time_in_ms *1000ul );
  delay(sleep_time_in_ms + 1ul); 
}

