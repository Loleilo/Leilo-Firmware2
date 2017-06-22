#include <Arduino.h>
#include <FS.h>
#include "debug.h"

JsonObject& configObj;

bool loadConfigFS() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    dp("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    dp("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  configObj = jsonBuffer.parseObject(buf.get());

  if (!configObj.success()) {
    dp("Failed to parse config file");
    return false;
  }

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
