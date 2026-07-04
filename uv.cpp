#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UrlEncode.h>
#include <ArduinoJson.h>
#include "uv.h"

String buildCurrentUVIndexEndpointURL(const double latitude, const double longitude, const String timezone) {
  String url = "https://uvindexapi.com/api/v1/forecast?latitude=";
  url += urlEncode(String(latitude, 6));
  url += "&longitude=";
  url += urlEncode(String(longitude, 6));
  url += "&timezone=";
  url += urlEncode(timezone);
  return url;
}

float fetchCurrentUVIndex(const double latitude, double longitude, const String timezone) {
  if (WiFi.status() != WL_CONNECTED) { 
    return NAN;
  }

  WiFiClientSecure wifiClient;
  wifiClient.setInsecure();

  HTTPClient httpClient;
  if (!httpClient.begin(wifiClient, buildCurrentUVIndexEndpointURL(latitude, longitude, timezone))) {
    return NAN;
  }

  int responseCode = httpClient.GET();
  httpClient.end();
  if (responseCode == 200) {
    JsonDocument uvIndexResponse;
    DeserializationError deserializationError = deserializeJson(uvIndexResponse, httpClient.getString());
    if (deserializationError) {
      return NAN; 
    };
    return (float) (uvIndexResponse["now"]["uv_index"] | NAN);
  }
  return NAN;
}
