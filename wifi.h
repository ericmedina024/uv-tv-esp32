#include <WiFi.h>

bool connectWiFi(const String &wifiSsid, const String &wifiPassword);
void broadcastWiFi(const String &wifiSsid, const String &wifiPassword, const IPAddress& arduinoIp, const IPAddress& arduinoNetMask);