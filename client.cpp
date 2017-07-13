#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <time.h>
#include "config.h"
#include "debug.h"
#include "pins.h"
#include "sleep.h"
#include "requests.h"
#include "datetime.h"

constexpr int maxTries = 40;

HTTPClient http;

String loginJSON, createGroupJSON, createAtomJSON;
const char * headerKeys[] = {"Set-Cookie"};
size_t headerkeyssize = sizeof(headerKeys) / sizeof(char*);

int* pollCnt;
bool hasHeartbeat;

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

bool clientSetup() {
  return true;
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

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");

  //  initSleep();
  dpnl("Connecting to ");
  dpnl(cfg.wifi.ssid);
  WiFi.setOutputPower(0);
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

  Serial.println("scan start");



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
        int res = sendRequest(createGroupJSON, (char*&) cfg.leilo.groupID);
        cfg.leilo.idAlloc = true;
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

  //init polling stuff
  pollCnt = new int[cfg.numAtoms];


  hasHeartbeat = false;

  StaticJsonBuffer<1024> jsonBuffer;
  for (int i = 0; i < cfg.numAtoms; i++) {
    pollCnt[i] = 0;
    Atom& atom = cfg.atoms[i];
    if (atom.poll == -1) continue;
    switch (atom.state) {
      case STATE_UNCREATED: {
          dpnl("Atom ");
          dpnl(atom.key);
          dpnl("is uncreated, must be recreated...");
          int res = sendRequest(createAtomJSON, (char*&)atom.id );
          atom.idAlloc = true;
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
            modified = true;
          } else {
            dpnl("Error code ");
            dp(res);
            return false;
          }
          break;
        }
    }

    //init sensor stuff
    switch (atom.type) {
      case TYPE_HEARTBEAT: {
          hasHeartbeat = true;
          break;
        }
      case TYPE_DIGITAL:
      case TYPE_ANALOG: {
          pinMode(atom.pin, atom.direction);
          break;
        }
    }
  }

  //check if config has changed, if so then save again
  if (modified) {
    dpnl("Modified. Need to resave...");
    if (!saveConfigFS()) {
      dp("Save config modifications unsuccessful.");
      return false;
    }
    dp("success");
  }

  //init time, if needed
  if (hasHeartbeat) {
    if (!initTime()) {
      dp("Could not start time. Disabling heartbeat temporarily");
      hasHeartbeat = false;
    }
  }
  return true;
}

bool clientLoop() {
  for (int i = 0; i < cfg.numAtoms; i++) {
    Atom& atom = cfg.atoms[i];
    if (atom.poll == -1)continue;//atom is disabled, skip it

    if (pollCnt[i] == 0) {
      dpnl("Sending atom ");
      dp(atom.key);
      int res;
      switch (atom.type) {
        case TYPE_HEARTBEAT: {
            if (hasHeartbeat) {
              time_t now = time(nullptr);
              res = sendRequest(createWriteAtomRequest(cfg.leilo.groupID, atom.id, ctime(&now)));
              if (res == 0) {
                dpnl("Heartbeat ");
                dpnl(atom.id);
                dp(" sent successfully");
                pollCnt[i] = atom.poll;
              } else {
                dpnl("Error sending heartbeat ");
                dp(atom.id);
              }
            }
            break;
          }
        case TYPE_DIGITAL: //handle both cases with one handler
        case TYPE_ANALOG: {
            if (atom.direction == INPUT || atom.direction == INPUT_PULLUP) {
              int sensorVal;
              if (atom.type == TYPE_DIGITAL)
                sensorVal = digitalRead(atom.pin);
              else
                sensorVal = analogRead(atom.pin);

              char buf[10];

              res = sendRequest(createWriteAtomRequest(cfg.leilo.groupID, atom.id, itoa(sensorVal, buf, 10)));
              if (res == 0) {
                dpnl("Sensor value ");
                dpnl(atom.key);
                dp(" sent successfully");
                pollCnt[i] = atom.poll;
              } else {
                dpnl("Error sending value ");
                dp(atom.key);
              }
            } else {
              //todo finish this
            }
            break;
          }
      }
      if (res == 4) {
        dp("deuathed. relogging in");
         if(!tryLogin()){
         dp("failed");
         res=-1;
         }
      }
      if(res!=0){
        return false;
      }
    } else --pollCnt[i];
  }
  delay(cfg.leilo.pollInt);
  return true;
}

bool clientCleanup() {
  WiFi.disconnect();
  if (cfg.leilo.idAlloc)
    delete[] cfg.leilo.groupID;
  for (int i = 0; i < cfg.numAtoms; i++) {
    if (cfg.atoms[i].idAlloc)
      delete[] cfg.atoms[i].id; // should be the only one that is dynamically allocated
  }
  delete[] pollCnt;
  return true;
}
