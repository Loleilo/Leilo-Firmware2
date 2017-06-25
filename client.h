#include <ESP8266HTTPClient.h>

extern HTTPClient http;
void clientSetup();
bool clientBegin();
void clientLoop();
void clientCleanup();
