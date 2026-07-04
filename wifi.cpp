#include <WiFi.h>
#include <Arduino.h>
#include "wifi.h"

bool connectWiFi(const String &wifiSsid, const String &wifiPassword) {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false);
  delay(1000);

  int currentAttempt = 1;
  int maxAttempts = 5;
  while (WiFi.status() != WL_CONNECTED) {
    if (currentAttempt > maxAttempts) {
      return false;
    }
    WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
    currentAttempt += 1;
    delay(10000);
  }
  return true;
}

void broadcastWiFi(const String &wifiSsid, const String &wifiPassword, const IPAddress& arduinoIp, const IPAddress& arduinoNetMask) {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(arduinoIp, arduinoIp, arduinoNetMask);
  WiFi.softAP(wifiSsid, wifiPassword);
}
