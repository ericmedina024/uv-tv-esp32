#include <Preferences.h>
#include <Arduino.h>
#include "config.h"

Preferences prefs;


bool isValidConfig(const Config& config) {
  return config.longitude != 0 && config.latitude != 0
    && config.wifiSsid.length() != 0 && config.wifiPassword.length() != 0;
}

Config loadConfig() {
  struct Config config;
  prefs.begin("uv-config", true);
  config.wifiSsid = prefs.getString("wifi_ssid", "");
  config.wifiPassword = prefs.getString("wifi_password", "");
  config.latitude = prefs.getDouble("latitude", 0);
  config.longitude = prefs.getDouble("longitude", 0);
  prefs.end();
  return config;
}


void saveConfig(const Config &config) {
  prefs.begin("uv-config", false);
  prefs.putString("wifi_ssid", config.wifiSsid);
  prefs.putString("wifi_password", config.wifiPassword);
  prefs.putDouble("latitude", config.latitude);
  prefs.putDouble("longitude", config.longitude);
  prefs.end();
}
