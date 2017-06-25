#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include "config.h"
#include "debug.h"
#include "pins.h"
#include "sleep.h"
#include "requests.h"

constexpr int maxTries = 40;

HTTPClient http;

String loginJSON, createGroupJSON, createAtomJSON;
const char * headerKeys[] = {"Set-Cookie"};
size_t headerkeyssize = sizeof(headerKeys) / sizeof(char*);

bool tryLogin() {
  int cnt = 0;
  while (true) {
    int res = sendRequest(loginJSON);
    if (res == 0) {
      if (http.hasHeader(headerKeys[0])) {
        cookies = http.header(headerKeys[0]);
        dpnl("Login successful. Cookies: ");
        dp(cookies);
        return true;
      }
    }

    cnt++;
    if (cnt >= maxTries) {
      dp("Client unable to begin: Could not login");
      return false;
    }
    delay(1400);
  }
}

void clientSetup() {
}

bool clientBegin() {
  // load settings
  dp("Try load settings from FS");
  if (!loadConfigFS())
  {
    dp("Client unable to begin: Could not load config");
    return false;
  }

  //init wifi
  WiFi.mode(WIFI_STA);
  initSleep();
  dpnl("Connecting to ");
  dpnl(cfg.wifi.ssid);
  WiFi.begin(cfg.wifi.ssid, cfg.wifi.password);

  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    dpnl(".");
    cnt++;
    if (cnt >= maxTries) {
      dp();
      dp("Client unable to begin: Could not connect to wifi");
      return false;
    }
  }
  dp();
  dp("Connected!");

  //init request stuff
  http.collectHeaders(headerKeys, headerkeyssize);
  loginJSON = createLoginRequest(cfg.leilo.username, cfg.leilo.password);
  createGroupJSON = createCreateGroupRequest();

  tryLogin();

  //apply modifications
  bool modified = true; //sketchy trick for modification checking
  switch (cfg.leilo.groupState) {
    case STATE_UNCREATED: {
        dpnl("Group object is uncreated, must be recreated...");
        String data;
        int res = sendRequest(createGroupJSON,(char*&) cfg.leilo.groupID);
        if (res == 0) {
          cfg.leilo.groupState = STATE_UNMODIFIED;
          dpnl("Success. Recieved group UUID ");
          dp(cfg.leilo.groupID);
        } else {
          dpnl("Error code ");
          dp(res);
          delete[] cfg.leilo.groupID;
          return false;
        }
      }//don't break, since we need to upload name too
    case STATE_MODIFIED: {
        dpnl("Group name has been changed, must be applied...");
        if (0 == strcmp(cfg.leilo.groupName, ""))
        {
          dp("Empty group name, not sending");
          break;
        }
        dp(cfg.leilo.groupID);
        int res = sendRequest(createSetGroupNameRequest(cfg.leilo.groupID, cfg.leilo.groupName));
        if (res == 0) {
          cfg.leilo.groupState = STATE_UNMODIFIED;
          dp("Success");
        } else {
          dpnl("Error code ");
          dp(res);
          return false;
        }
        break;
      }
    default: {
        modified = false; //sketchy trick for modification checking
      }
  }

  createAtomJSON = createCreateAtomRequest(cfg.leilo.groupID);

  StaticJsonBuffer<1024> jsonBuffer;
  for (int i = 0; i < cfg.numAtoms; i++) {
    Atom& atom = cfg.atoms[i];
    switch (atom.state) {
      case STATE_UNCREATED: {
          dpnl("Atom ");
          dpnl(atom.key);
          dpnl("is uncreated, must be recreated...");
          int res = sendRequest(createAtomJSON, (char*&)atom.id );
          if (res == 0) {
            atom.state = STATE_UNMODIFIED;
            modified = true;
            dpnl("Success. Recieved atom UUID ");
            dp(atom.id);
          } else {
            dpnl("Error code ");
            dp(res);
            delete[] atom.id;
            return false;
          }
        }//don't break, since we need to upload info too
      case STATE_MODIFIED: {
          dpnl("Atom ");
          dpnl(atom.name);
          dpnl("is modified, changes being applied to server...");
          if (0 == strcmp(atom.name, ""))
          {
            dp("Empty group name, not sending");
            break;
          }
          int res = sendRequest(createSetAtomNameRequest(cfg.leilo.groupID, atom.id, atom.name));
          if (res == 0) {
            cfg.leilo.groupState = STATE_UNMODIFIED;
            dp("Success");
          } else {
            dpnl("Error code ");
            dp(res);
            return false;
          }
          break;
        }
    }
  }

  if (modified) {
    dpnl("Modified. Need to resave...");
    if (!saveConfigFS()) {
      dp("Save config modifications unsuccessful.");
      return false;
    }
    dp("success");
  }

  return true;
}

void clientLoop() {

}

void clientCleanup() {
  WiFi.disconnect();
}
