#include <ESP8266WebServer.h>
#include "config.h"
#include <FS.h>
#include "debug.h"

constexpr char * WiFiAPPSK = "password";

ESP8266WebServer server(80);    //listen on port 80

void setconfigHandler() {
  if(saveConfigStr(server.arg("plain"))){
    server.send(200, "Sucess saving");
  }else{
    server.send(500, "Error saving");
  }
}

void serverSetup() {
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/getconfig", SPIFFS, "/config.json");
  server.on("/setconfig", setconfigHandler);
}

bool serverBegin() {
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "sensor-" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);
  
  if (!WiFi.softAP(AP_NameChar, WiFiAPPSK)){
     Serial.println("Error starting soft AP");
    return false;
  }
  dpnl("Soft AP started sucessfully at: ");
  dp(WiFi.softAPIP());

  server.begin();
  return true;
}

void serverLoop() {
  server.handleClient();
}

bool serverCleanup() {
  dp("Server cleaning up...");
  if(!WiFi.softAPdisconnect(true)){
    dp("Disconnect failed");
    return false;
  }
  dp("Soft AP disconnected");
  WiFi.disconnect();
  dp("Done");
}
