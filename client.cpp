#include <ESP8266WiFi.h>
#include "config.h"
#include "debug.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

const int maxTries=40;

void clientSetup() {
  
}

bool clientBegin() {
  dp("Try load settings from FS");
  if(!loadConfigFS())
  {
    dp("Client unable to begin: Could not load config");
    return false;
  }

  WiFi.mode(WIFI_STA);
  
  WiFi.begin(configuration.ssid,configuration.wifiPassword);

  dpnl("Connecting to ");
  dp(configuration.ssid);

  int cnt=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    dpnl(".");
    cnt++;
    if(cnt>=maxTries){
      dp("Client unable to begin: Could not connect to wifi");
      return false;
    }
  }
  dp("Connected!");
  return true;
}

void clientLoop() {
}

void clientCleanup() {
}
