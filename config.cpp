#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "debug.h"
#include "config.h"

Config configuration;

const int BUF_SIZE=1024;

char buf[BUF_SIZE];

bool loadConfigFS() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    dp("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > BUF_SIZE) {
    dp("Config file size is too large" + String(size));
    return false;
  }

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf, size);

  StaticJsonBuffer<BUF_SIZE> jsonBuffer;
  JsonObject& configObj = jsonBuffer.parseObject(buf);

  if (!configObj.success()) {
    dp("Failed to parse config file");
    return false;
  }

  configuration.ssid = configObj["wifi"]["ssid"];
  configuration.wifiPassword = configObj["wifi"]["password"];

  JsonObject& leiloObj = configObj["leilo"];
  configuration.apiURL = leiloObj["apiURL"];
  configuration.apiVersion = leiloObj["version"];
  configuration.username = leiloObj["username"];
  configuration.leiloPassword = leiloObj["password"];
  configuration.groupID = leiloObj["groupID"];
  configuration.atomID = leiloObj["atomID"];
  
  return true;
}

bool saveConfigStr(String cfg) {
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    dp("Failed to open config file for writing");
    return false;
  }

  configFile.print(cfg);
  return true;
}
