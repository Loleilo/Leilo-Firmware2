#include <Arduino.h>
#include <FS.h>

bool saveConfigStr(String cfg){
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  configFile.print(cfg);
  return true;
}
