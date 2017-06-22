#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "server.h"
#include "client.h"
#include "config.h"

#define DEBUG

const int modePin = 13;

bool serverMode = true;
bool modeChanged = false;

//TODO needs to be put in server file

void modePinChanged() {
  modeChanged = true;
}

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif

  //setup fs
  SPIFFS.begin();

  //other stuff
  serverSetup();
  clientSetup();

  //setup hardware
  pinMode(modePin, INPUT_PULLUP);
  serverMode = digitalRead(modePin);
  attachInterrupt(digitalPinToInterrupt(modePin), modePinChanged, CHANGE);

  serverBegin();
}

void loop() {
  //  if (modeChanged) {
  //    modeChanged = false;
  //    bool prevMode = serverMode;
  //    serverMode = digitalRead(modePin);
  //    if (serverMode != prevMode) {
  //      if (serverMode) {
  //        clientCleanup();
  //        serverBegin();
  //      } else {
  //        serverCleanup();
  //        clientBegin();
  //      }
  //    }
  //  }
  //  if (serverMode) {
  //    serverLoop();
  //  } else {
  //    clientLoop();
  //  }
  serverLoop();
}
