#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "server.h"
#include "client.h"
#include "config.h"
#include "debug.h"

#define DEBUG

const int debounce = 200;
const int modePin = 13;

bool modeChanged = false;
bool serverMode = true;

unsigned long last_interrupt_time = 0;

void modePinChanged() {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > debounce) {
    modeChanged = true;
  }
  last_interrupt_time = interrupt_time;
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
  attachInterrupt(digitalPinToInterrupt(modePin), modePinChanged, RISING);

  serverBegin();
}

void loop() {
  if (modeChanged) {
    modeChanged = false;
    serverMode = !serverMode;
    if (serverMode) {
      dp("Server mode on");
      clientCleanup();
      serverBegin();
    } else {
      dp("Server mode off");
      serverCleanup();
      clientBegin();
    }
  }
  if (serverMode) {
    serverLoop();
  } else {
    clientLoop();
  }
  serverLoop();
}
