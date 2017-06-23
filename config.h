struct Atom{
  
}

struct ConfigT {
  struct {
    const char* ssid;
    const char* password;
  } wifi;
  struct {
    const char* apiVersion;
    const char* apiURL;
    const char* username;
    const char* password;
    const char* groupID;
  } leilo;
  int numAtoms;
  Atom* atoms;
};
typedef ConfigT Config;

extern Config configuration;

bool loadConfigFS();
bool saveConfigStr(String cfg);
