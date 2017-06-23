struct ConfigT{
  const char* ssid;
  const char* wifiPassword;
};
typedef ConfigT Config;

extern Config configuration;

bool loadConfigFS();
bool saveConfigStr(String cfg);
