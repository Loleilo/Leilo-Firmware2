#include <ArduinoJson.h>
#include "client.h"

extern String cookies;

template<unsigned int n>
String createRequest(StaticJsonBuffer<n> &buf, const char* call, JsonObject& params) {
  JsonObject& root = buf.createObject();
  root["version"] = cfg.leilo.apiVersion;
  root["call"] = call;
  root["params"] = params;
  String res;
  root.printTo(res);
  return res;
}

String createCreateGroupRequest();
String createLoginRequest( const char* const &user, const char* const &pass);
String createSetGroupNameRequest( const char* const &groupID, const char* const &name);
String createSetAtomNameRequest( const char* const &groupID, const char* const &atomID, const char* const &name);
String createWriteAtomRequest( const char* const &groupID, const char* const &atomID, const char* const &value);

String createCreateAtomRequest( const char* const &groupID);

int sendRequest(String req);
int sendRequest(String req,  char* &data);
