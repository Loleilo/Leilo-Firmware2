#ifndef _CONFIG_H
#define _CONFIG_H
struct AtomT {
  const char* key;
  const char* id ;
  bool idAlloc=false;
  const char* name ;
  int state;//1 for unmodified, 2 for modified, 3 for uncreated
  int type; //1 for digital, 2 for analog, 3 for heartbeat
  int direction; //use default consts
  int poll;
  int pin;
};
typedef AtomT Atom;

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
  bool idAlloc=false;
    const char* groupName;
    int groupState;
    int pollInt;
  } leilo;
  int numAtoms;
  Atom* atoms;
};
typedef ConfigT Config;

extern Config cfg;
constexpr int TYPE_DIGITAL = 1;
constexpr int TYPE_ANALOG = 2;
constexpr int TYPE_HEARTBEAT = 3;
constexpr int STATE_UNMODIFIED = 1;
constexpr int STATE_MODIFIED = 2;
constexpr int STATE_UNCREATED = 3;

bool loadConfigFS();
bool saveConfigFS();
bool saveConfigStr(String cfg);
#endif
