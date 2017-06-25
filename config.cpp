#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "debug.h"
#include "config.h"

Config cfg;

constexpr int BUF_SIZE = 1024;

char buf[BUF_SIZE];

int stateStrToInt(const char* const &str) {
  if (0 == strcmp(str ,  "unmodified")) {
    return STATE_UNMODIFIED;
  } else if (0 == strcmp(str ,  "modified")) {
    return STATE_MODIFIED;
  } else if (0 == strcmp(str , "uncreated")) {
    return STATE_UNCREATED;
  } else {
    dp("Error! Unknown state in config.");
    return -1;
  }
}

const char* stateIntToStr(int state) {
  switch (state) {
    case STATE_UNMODIFIED:
      return "unmodified";
    case STATE_MODIFIED:
      return "modified";
    case STATE_UNCREATED:
      return "uncreated";
  }
}

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

  //  dp("Config obj:");
  //  dp(buf);

  StaticJsonBuffer<BUF_SIZE> jsonBuffer;
  JsonObject& configObj = jsonBuffer.parseObject(buf);

  if (!configObj.success()) {
    dp("Failed to parse config file");
    return false;
  }


  cfg.wifi.ssid = configObj["wifi"]["ssid"];
  cfg.wifi.password = configObj["wifi"]["password"];

  JsonObject& leiloObj = configObj["leilo"];
  cfg.leilo.apiURL = leiloObj["apiURL"];
  cfg.leilo.apiVersion = leiloObj["version"];
  cfg.leilo.username = leiloObj["username"];
  cfg.leilo.password = leiloObj["password"];
  cfg.leilo.groupID = leiloObj["groupID"];
  cfg.leilo.groupName = leiloObj["groupName"];
  cfg.leilo.groupState = stateStrToInt(leiloObj["groupState"]);

  cfg.numAtoms = configObj["numAtoms"];
  delete[] cfg.atoms;
  dpnl("Number of atoms to load: ");
  dp(cfg.numAtoms);

  cfg.atoms = new Atom[cfg.numAtoms];
  int idx = 0;
  JsonObject& atomsObj = configObj["atoms"];
  for (JsonObject::iterator it = atomsObj.begin(); it != atomsObj.end(); ++it)
  {
    Atom& atom = cfg.atoms[idx];
    atom.key = it->key;
    JsonObject& atomObj = it->value;
    atom.id = atomObj["id"];
    atom.name = atomObj["name"];
    atom.state = stateStrToInt(atomObj["state"]);

    const char* const &atomType = atomObj["type"];
    if (0 == strcmp(atomType, "digital")) {
      atom.type = TYPE_DIGITAL;
    } else if (0 == strcmp(atomType, "analog")) {
      atom.type = TYPE_ANALOG;
    } else if (0 == strcmp(atomType, "heartbeat")) {
      atom.type = TYPE_HEARTBEAT;
    } else {
      dp("Error! Unknown type in config.");
    }
    if (atom.type != 3) {
      const char*  const &direction = atomObj["direction"];
      if ( 0 == strcmp (direction , "input")) {
        atom.direction = INPUT;
      } else  if ( 0 == strcmp (direction , "input_pullup")) {
        atom.direction = INPUT_PULLUP;
      } else  if ( 0 == strcmp (direction , "output")) {
        atom.direction = OUTPUT;
      } else {
        dp("Error! Unknown direction in config.");
      }
    }

    atom.poll = atomObj["poll"];
    idx++;
  }
  return true;
}

bool saveConfigFS() {
  StaticJsonBuffer<BUF_SIZE> jsonBuffer;
  JsonObject& configObj = jsonBuffer.createObject();
  JsonObject& wifi = jsonBuffer.createObject();
  wifi["ssid"] = cfg.wifi.ssid;
  wifi["password"] = cfg.wifi.password;
  configObj["wifi"] = wifi;

  JsonObject& leiloObj = configObj.createNestedObject("leilo");
  leiloObj["apiURL"] = cfg.leilo.apiURL;
  leiloObj["version"] = cfg.leilo.apiVersion;
  leiloObj["username"] = cfg.leilo.username;
  leiloObj["password"] = cfg.leilo.password;
  leiloObj["groupID"] = cfg.leilo.groupID;
  leiloObj["groupName"] = cfg.leilo.groupName;
  leiloObj["groupState"] = stateIntToStr(cfg.leilo.groupState);

  configObj["numAtoms"] = cfg.numAtoms;

  JsonObject& atomsObj = configObj.createNestedObject("atoms");
  for (int idx = 0; idx < cfg.numAtoms; idx++)
  {
    Atom& atom = cfg.atoms[idx];
    JsonObject& atomObj = atomsObj.createNestedObject(atom.key);
    atomObj["id"] = atom.id;
    atomObj["name"] = atom.name;
    atomObj["state"] = stateIntToStr(atom.state);

    if (atom.type == TYPE_DIGITAL) {
      atomObj["type"] = "digital";
    } else if (atom.type == TYPE_ANALOG) {
      atomObj["type"] = "analog";
    } else if (atom.type == TYPE_HEARTBEAT) {
      atomObj["type"] = "heartbeat";
    }

    if (atom.type == TYPE_DIGITAL || atom.type == TYPE_ANALOG) {
      if ( atom.direction == INPUT) {
        atomObj["direction"] = "input";
      } else if (atom.direction == INPUT_PULLUP) {
        atomObj["direction"] = "input_pullup";
      } else if (atom.direction == OUTPUT) {
        atomObj["direction"] = "output";
      } else {
        dp("Error! Unknown direction in config.");
      }
    }

    atomObj["poll"] = atom.poll;
  }

  String cfgStr;
  configObj.printTo(cfgStr);
  saveConfigStr(cfgStr);
  return true;
}

bool saveConfigStr(String cfgStr) {
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    dp("Failed to open config file for writing");
    return false;
  }

  configFile.print(cfgStr);
  return true;
}
