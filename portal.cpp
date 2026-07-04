#include <WebServer.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>
#include "config.h"
#include "portal.h"

Config hostConfigurationServer(const Config &currentConfig) {
  bool stopServer = false;
  struct Config newConfig;
  WebServer server(80);
  server.on("/", HTTP_GET, [&server]() {
    server.send(200, "text/html", INDEX_HTML);
  });
  server.on("/", HTTP_POST, [&server, &newConfig]() {
    const String wifiSsid = server.arg("wifi_network");
    if (wifiSsid.length() == 0) {
      server.sendHeader("Location", "/?error=" + urlEncode("Error: Invalid WiFi network"));
      server.send(303);
      return;
    }
    const String wifiPassword = server.arg("wifi_password");
    if (wifiPassword.length() == 0) {
      server.sendHeader("Location", "/?error=" + urlEncode("Error: Invalid WiFi password"));
      server.send(303);
      return;
    }
    const String latitude = server.arg("latitude");
    const double latitudeDouble = latitude.toDouble();
    if (latitudeDouble == 0.0) {
      server.sendHeader("Location", "/?error=" + urlEncode("Error: Invalid latitude"));
      server.send(303);
      return;
    }
    const String longitude = server.arg("longitude");
    const double longitudeDouble = longitude.toDouble();
    if (longitudeDouble == 0.0) {
      server.sendHeader("Location", "/?error=" + urlEncode("Error: Invalid longitude"));
      server.send(303);
      return;
    }
    newConfig.latitude = latitudeDouble;
    newConfig.longitude = longitudeDouble;
    newConfig.wifiPassword = wifiPassword;
    newConfig.wifiSsid = wifiSsid;
    server.sendHeader("Location", "/success");
    server.send(303);
  });
  server.on("/current-config", [&server, &currentConfig]() {
    JsonDocument currentConfigJson;
    currentConfigJson["latitude"] = currentConfig.latitude;
    currentConfigJson["longitude"] = currentConfig.longitude;
    currentConfigJson["wifiSsid"] = currentConfig.wifiSsid;
    currentConfigJson["wifiPassword"] = currentConfig.wifiPassword;
    String serializedConfig;
    serializeJson(currentConfigJson, serializedConfig);
    server.send(200, "application/json", serializedConfig);
  });
  server.on("/success", HTTP_GET, [&server, &stopServer, &newConfig]() {
    if (!isValidConfig(newConfig)) {
      server.sendHeader("Location", "/?error=" + urlEncode("Error: Config is not valid"));
      server.send(303);
      return;
    }
    server.send(200, "text/html", SUCCESS_HTML);
    stopServer = true;
  });
  server.begin();
  while (!stopServer) {
    server.handleClient();
  }
  server.stop();
  return newConfig;
}
