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

float fetchCurrentUVIndex(const double latitude, double longitude, const String timezone, String& outError) {
  if (WiFi.status() != WL_CONNECTED) { 
    outError = "WiFi not connected";
    return NAN;
  }

  WiFiClientSecure wifiClient;
  wifiClient.setInsecure();

  HTTPClient httpClient;
  if (!httpClient.begin(wifiClient, buildCurrentUVIndexEndpointURL(latitude, longitude, timezone))) {
    outError = "Connection to UV service failed";
    return NAN;
  }

  int responseCode = httpClient.GET();
  String uvIndexReponseString = httpClient.getString();
  httpClient.end();
  if (responseCode == 200) {
    JsonDocument uvIndexResponse;
    DeserializationError deserializationError = deserializeJson(uvIndexResponse, uvIndexReponseString);
    if (deserializationError) {
      outError = "UV deserialization failed";
      return NAN;
    };
    if (!uvIndexResponse.containsKey("now")) {
      outError = "Key 'now' missing";
      return NAN;
    }
    auto now = uvIndexResponse["now"];
    if (!now.is<JsonObject>()) {
      outError = "Key 'now' not an object";
      return NAN;
    } else if (!now.containsKey("uv_index")) {
      outError = "Key 'uv_index' missing";
      return NAN;
    }
    float uv = now["uv_index"].as<float>();
    if (!isfinite(uv)) {
      outError = "Invalid 'uv_index' value";
      return NAN;
    }
    return uv;
  }
  outError = "Received " + String(responseCode) + " response";
  return NAN;
}
