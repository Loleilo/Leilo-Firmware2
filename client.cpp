#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include "config.h"
#include "debug.h"
#include "pins.h"
#include "sleep.h"

const int maxTries = 40;
const int readPin = D6;

volatile bool updated = false;
bool nextval;
HTTPClient http;

String loginJSON;
String cookies;
const char * headerKeys[] = {"Set-Cookie"};
size_t headerkeyssize = sizeof(headerKeys) / sizeof(char*);

//ICACHE_RAM_ATTR void pinChanged() {
//  updated = true;
//}

int login() {
  http.begin(configuration.apiURL);
  dp("Sending login" + loginJSON);
  http.POST(loginJSON);
  String res = http.getString();

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(res);
  if (root["returnCode"] == 0) {
    if (http.hasHeader(headerKeys[0])) {
      cookies = http.header(headerKeys[0]);

      return 0;
    }
    else {
      return -1;
    }
  }

  return root["returnCode"];
}

void clientSetup() {
  pinMode(readPin, INPUT_PULLUP);
}

bool clientBegin() {
  dp("Try load settings from FS");
  if (!loadConfigFS())
  {
    dp("Client unable to begin: Could not load config");
    return false;
  }

  WiFi.mode(WIFI_STA);
  initSleep();

  WiFi.begin(configuration.ssid, configuration.wifiPassword);

  dpnl("Connecting to ");
  dp(configuration.ssid);

  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    dpnl(".");
    cnt++;
    if (cnt >= maxTries) {
      dp("Client unable to begin: Could not connect to wifi");
      return false;
    }
  }
  dp("Connected!");

  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["version"] = configuration.apiVersion;
  root["call"] = "login";

  JsonObject& params = jsonBuffer.createObject();
  params["username"] = configuration.username;
  params["password"] = configuration.leiloPassword;
  root["params"] = params;

  loginJSON = "";
  root.printTo(loginJSON);

  http.collectHeaders(headerKeys, headerkeyssize);
  while (true) {
    int res = login();

    if (res == 0) {
      dp("login success");
      dp("cookie " + cookies);
      break;
    } else {
      dp("login fail code " + String(res));
    }
    if (cnt >= maxTries) {
      dp("Client unable to begin: Could not login");
      return false;
    }
    delay(1400);
  }

//  attachInterrupt(digitalPinToInterrupt(readPin), pinChanged, CHANGE);

  return true;
}

void clientLoop() {
//  if (updated) {
    nextval = digitalRead(readPin);
    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["version"] = configuration.apiVersion;
    root["call"] = "writeAtom";

    JsonObject& params = jsonBuffer.createObject();
    params["group_id"] = configuration.groupID;
    params["atom_id"] = configuration.atomID;
    params["value"] = (nextval ? "high" : "low");
    root["params"] = params;
    String out;
    root.printTo(out);
    dp("sending: " + out);
    http.begin(configuration.apiURL);
    http.addHeader("Cookie", cookies);
    http.POST(out);
    String res = http.getString();
    if (res.length() == 0) {
      dp("Error: server gave empty response");
      return;
    }
    dp("result is" + res);
    JsonObject& root2 = jsonBuffer.parseObject(res);
    int code = root2["returnCode"];
    if (code == 0) {
      dp("succes");
      updated = false;

    } else if (code == 4) {

      dp("de authed, relogging in");
      if (login() == 0)

        dp("succes");
      else
        dp("errror");
    } else {

      dp("errror");
    }
//  }
  delay(1000);

}

void clientCleanup() {
  detachInterrupt(digitalPinToInterrupt(readPin));
  WiFi.disconnect();
}
