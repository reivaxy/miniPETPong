
#include "Config.h"

Config::Config(unsigned int version, const char* type, const char* name):XEEPROMConfigClass(version + CONFIG_BASE_VERSION, type, sizeof(ConfigStruct)) {
  strlcpy(_defaultModuleName, name, NAME_MAX_LENGTH + 1);
  setName(name);
}

Config::Config(unsigned int version, const char* type, const char* name, unsigned int dataSize):XEEPROMConfigClass(version + CONFIG_BASE_VERSION, type, dataSize) {
  strlcpy(_defaultModuleName, name, NAME_MAX_LENGTH + 1);
  setName(name);
}

/**
* Reset the config data structure to the default values.
* This is done each time the data structure version is different from the one saved in EEPROM
* NB: version and name are handled by base class
*/
void Config::initFromDefault() {
  XEEPROMConfigClass::initFromDefault(); // handles version and name init
  ConfigStruct* configPtr = _getDataPtr();
  
  setName(_defaultModuleName); // Reset module name to default name
  setAPSsid(DEFAULT_AP_SSID);
  setAPPwd(DEFAULT_AP_PWD);
  setHomeSsid("");
  setHomePwd("");
}


void Config::setName(const char* name) {
  strlcpy(_getDataPtr()->moduleName, name, NAME_MAX_LENGTH + 1);
}
const char* Config::getName(void) {
  return _getDataPtr()->moduleName;
}

void Config::setAPSsid(const char* ssid) {
  strlcpy(_getDataPtr()->APSsid, ssid, SSID_MAX_LENGTH + 1);
}
const char* Config::getAPSsid() {
  return _getDataPtr()->APSsid;
}

void Config::setAPPwd(const char* pwd) {
  strlcpy(_getDataPtr()->APPwd, pwd, PWD_MAX_LENGTH + 1);
}
const char* Config::getAPPwd() {
  return _getDataPtr()->APPwd;
}

void Config::setHomeSsid(const char* ssid) {
  strlcpy(_getDataPtr()->ssid, ssid, SSID_MAX_LENGTH + 1);
}
const char* Config::getHomeSsid() {
  return _getDataPtr()->ssid;
}

void Config::setHomePwd(const char* pwd) {
  strlcpy(_getDataPtr()->pwd, pwd, PWD_MAX_LENGTH + 1);
}
const char* Config::getHomePwd() {
  return _getDataPtr()->pwd;
}

ConfigStruct* Config::_getDataPtr(void) {
  return (ConfigStruct*)XEEPROMConfigClass::_getDataPtr();
}