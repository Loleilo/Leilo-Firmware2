struct ConfigT{
  
  const char* ssid;
  const char* wifiPassword;
  const char* apiVersion;
  const char* apiURL;
  const char* username;
  const char* leiloPassword;
  const char* atomID;
  const char* groupID;
};
typedef ConfigT Config;

extern Config configuration;

bool loadConfigFS();
bool saveConfigStr(String cfg);
