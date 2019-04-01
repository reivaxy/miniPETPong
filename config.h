#pragma once

#include <XEEPROMConfig.h>

// Increment this when new fields (or new default values) are added to configStruct
// The module will then use default values, and will need to be configured again.
#define CONFIG_BASE_VERSION 2

// When not initialized yet, the esp module will open a Wifi access point with these:
#define DEFAULT_AP_SSID "miniPETPong"
#define DEFAULT_AP_PWD "miniPETPong"

#define NAME_MAX_LENGTH 20
#define SSID_MAX_LENGTH 20
#define PWD_MAX_LENGTH 50

// Fixed size structure to store configuration data that needs to be persisted (eeprom)
struct ConfigStruct:public XEEPROMConfigDataStruct {
  char moduleName[NAME_MAX_LENGTH + 1];  // Name of the module

  char APSsid[SSID_MAX_LENGTH + 1];
  char APPwd[PWD_MAX_LENGTH + 1];

  char ssid[SSID_MAX_LENGTH + 1];
  char pwd[PWD_MAX_LENGTH + 1];
};

class Config:public XEEPROMConfigClass {
public:
  Config(unsigned int version, const char* type, const char* moduleName);
  Config(unsigned int version, const char* type, const char* moduleName, unsigned int dataSize);
  virtual void initFromDefault();

  void setName(const char*);
  const char* getName(void);

  void setAPSsid(const char* ssid);
  const char* getAPSsid(void);

  void setAPPwd(const char* pwd);
  const char* getAPPwd(void);

  void setHomeSsid(const char* ssid);
  const char* getHomeSsid(void);

  void setHomePwd(const char* pwd);
  const char* getHomePwd(void);

protected:
  ConfigStruct* _getDataPtr(void);
  char _defaultModuleName[NAME_MAX_LENGTH + 1];  // to be able to reset it if needed
};
