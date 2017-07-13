#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "server.h"
#include "client.h"
#include "config.h"
#include "debug.h"
#include "pins.h"

//device will reboot if it gets stuck in error state for long than this time
constexpr int errorTimeout = 20000;

constexpr int debounce = 300;
constexpr int modePin = D7;
constexpr int modeIndPin = D1;
constexpr int maxErr = 50;

int serverMode;
bool err = true;

unsigned long last_interrupt_time = 0;
unsigned long lastError = 0;

int errCnt = 0;

bool initMode() {
  serverMode = digitalRead(modePin);
  if (serverMode) {
    dp("Server mode on");
    if (!clientCleanup()) {
      dp("Client cleanup failed.");
      return false;
    }
    if (!serverBegin()) {
      dp("Server begin failed.");
      return false;
    }
    digitalWrite(modeIndPin, HIGH);
  } else {
    dp("Server mode off");
    if (!serverCleanup()) {
      dp("Server cleanup failed.");
      return false;
    }
    if (!clientBegin()) {
      dp("Client begin failed.");
      return false;
    }
    digitalWrite(modeIndPin, LOW);
  }
  return true;
}

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  //  Serial.setDebugOutput(true); too much
#endif

  //setup fs
  SPIFFS.begin();

  //other stuff
  if (!serverSetup()) {
    dp("Server setup failed.");
    err = true;
    lastError = millis();
    return;
  }
  if (!clientSetup()) {
    err = true;
    lastError = millis();
    return;
  }

  //setup hardware
  pinMode(modePin, INPUT_PULLUP);
  pinMode(modeIndPin, OUTPUT);
  digitalWrite(modeIndPin, HIGH);

  //init mode
  err = initMode();
  if (err)
    lastError = millis();
}

void loop() {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > debounce) {
    int prevMode = serverMode;
    serverMode = digitalRead(modePin);
    if (prevMode != serverMode) {
      err = initMode();
      if (err)
        lastError = millis();
    }
    last_interrupt_time = interrupt_time;
  }
  else {
    if (!err) {
      if (millis() - lastError > errorTimeout)
        ESP.restart();
      digitalWrite(modeIndPin, HIGH);
      delay(500);
      digitalWrite(modeIndPin, LOW);
      delay(500);
    } else {
      if (serverMode) {
        serverLoop();
      } else {
        if (clientLoop()) {
          errCnt = 0;
        } else {
          errCnt++;
          if (errCnt > maxErr) {
            dp("Too many client errors occured in a row. Ending client loop");
            err = true;
          }
        }
      }
    }
  }
}
