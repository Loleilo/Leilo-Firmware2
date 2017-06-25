#include <ArduinoJson.h>
#include "config.h"
#include "client.h"
#include "debug.h"
#include "requests.h"

String cookies;


String createLoginRequest( const char* const &username, const char* const &password) {
  StaticJsonBuffer<300> buf;
  JsonObject& params = buf.createObject();
  params["username"] = username;
  params["password"] = password;
  return createRequest(buf, "login", params);
}

String createCreateGroupRequest() {
  StaticJsonBuffer<100> buf;
  JsonObject& params = buf.createObject();
  return createRequest(buf, "createGroup", params);
}

String createSetGroupNameRequest( const char* const &groupID, const char* const &name) {
  StaticJsonBuffer<200> buf;
  JsonObject& params = buf.createObject();
  params["group_id"] = groupID;
  params["name"] = name;
  return createRequest(buf, "setGroupName", params);
}

String createCreateAtomRequest( const char* const &groupID) {
  StaticJsonBuffer<200> buf;
  JsonObject& params = buf.createObject();
  params["group_id"] = groupID;
  return createRequest(buf, "createAtom", params);
}

String createSetAtomNameRequest( const char* const &groupID, const char* const &atomID, const char* const &name) {
  StaticJsonBuffer<300> buf;
  JsonObject& params = buf.createObject();
  params["group_id"] = groupID;
  params["atom_id"] = atomID;
  params["name"] = name;
  return createRequest(buf, "setAtomName", params);
}

int sendRequest(String req,  char*  &data) {
  http.begin(cfg.leilo.apiURL);
  dp("Sending request: " + req);
  http.addHeader("Cookie", cookies);
  http.POST(req);
  String res = http.getString();

  if (res.length() == 0) {
    dp("ERROR: Empty response");
    return -1;
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(res);

 const char* tmp=root["returnData"];
  data=new char[strlen(tmp)+1];
  strcpy(data,tmp);
  if (root["returnCode"] != 0) {
    dpnl("Server error: ");
    dp(data);
  }
  return root["returnCode"];
}

int sendRequest(String req) {
  http.begin(cfg.leilo.apiURL);
  dp("Sending request: " + req);
  http.addHeader("Cookie", cookies);
  http.POST(req);
  String res = http.getString();

  if (res.length() == 0) {
    dp("ERROR: Empty response");
    return -1;
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(res);


  if (root["returnCode"] != 0) {
    dpnl("Server error: ");
    const char* err=root["returnData"];
    dp(err);
  }
  return root["returnCode"];
}

