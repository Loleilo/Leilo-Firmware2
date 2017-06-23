#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "server.h"
#include "client.h"
#include "config.h"
#include "debug.h"
#include "pins.h"


const int debounce = 300;
const int modePin = D7;
const int modeIndPin = D1;

volatile bool modeChanged = true;
bool serverMode = true;

unsigned long last_interrupt_time = 0;

ICACHE_RAM_ATTR void modePinChanged() {
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
  pinMode(modeIndPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(modePin), modePinChanged, CHANGE);
}

void loop() {
  if (modeChanged) {
    unsigned long interrupt_time = millis();
    modeChanged = false;
    if (interrupt_time - last_interrupt_time > debounce) {
      serverMode = digitalRead(modePin);
      if (serverMode) {
        dp("Server mode on");
        clientCleanup();
        digitalWrite(modeIndPin, HIGH);
        serverBegin();
      } else {
        dp("Server mode off");
        if (!serverCleanup()) {
          dp("Server cleanup failed. Falling back to server mode");
          modeChanged = true;
          last_interrupt_time = 0;
          return;
        }
        digitalWrite(modeIndPin, LOW);
        if (!clientBegin()) {
          dp("Client begin failed. Falling back to server mode");
          modeChanged = true;
          last_interrupt_time = 0;
          return;
        }
      }
    }
    last_interrupt_time = interrupt_time;
  } else {
    if (serverMode) {
      serverLoop();
    } else {
      clientLoop();
    }
  }
}
