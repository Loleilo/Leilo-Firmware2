#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "server.h"
#include "client.h"
#include "config.h"
#include "debug.h"
#include "pins.h"

constexpr int debounce = 300;
constexpr int modePin = D7;
constexpr int modeIndPin = D1;

int serverMode;

unsigned long last_interrupt_time = 0;

void initMode() {
  serverMode = digitalRead(modePin);
  if (serverMode) {
    dp("Server mode on");
    clientCleanup();
    serverBegin();
    digitalWrite(modeIndPin, HIGH);
  } else {
    dp("Server mode off");
    if (!serverCleanup()) {
      dp("Server cleanup failed.");
      return;
    }
    if (!clientBegin()) {
      dp("Client begin failed.");
      return;
    }
    digitalWrite(modeIndPin, LOW);
  }
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
  pinMode(modeIndPin, OUTPUT);
  digitalWrite(modeIndPin, HIGH);

  //init mode
  initMode();
}

void loop() {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > debounce) {
    int prevMode = serverMode;
    serverMode = digitalRead(modePin);
    if (prevMode != serverMode) {
      initMode();
    }
    last_interrupt_time = interrupt_time;
  }
  else {
    if (serverMode) {
      serverLoop();
    } else {
      clientLoop();
    }
  }
}
