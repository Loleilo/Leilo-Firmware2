#include <ESP8266WiFi.h>
#include <time.h>
#include "debug.h"

constexpr int maxTries = 20;
constexpr int timezone = 3;
constexpr int dst = 0;

bool initTime() {
  configTime(timezone * 3600, dst, "pool.ntp.org", "time.nist.gov");
  dpnl("Waiting for time");
  int cnt=0;
  while (!time(nullptr)) {
    dpnl(".");
    cnt++;
    if(cnt>=maxTries){
      dp("Error connecting to time");
      return false;
    }
    delay(1000);
  }
  dp();
  dp("Success");
  return true;
}

