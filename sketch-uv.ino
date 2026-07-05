#include <SPI.h>
#include <TFT_eSPI.h>
#include "uv.h"
#include "config.h"
#include "wifi.h"
#include "state.h"
#include "sun.h"
#include "portal.h"
#include "esp_system.h"


#define DEBUG false
#define DEBUG_ALWAYS_START_PORTAL false

#define TFT_PWER_PIN 21
#define TC_PIN 32


TFT_eSPI tft = TFT_eSPI();

struct Config config;
State currentState;
long lastUVFetchMs = -1;
const uint32_t BACKGROUND_COLOR = 0x4e7b;
const unsigned long UV_UPDATE_INTERVAL_IN_MS = 5UL * 60UL * 1000UL;
const unsigned long RESTART_INTERVAL = 24UL * 60UL * 60UL * 1000UL;
const unsigned long EXTENDED_TOUCH_DURATION_IN_MS = 10UL * 1000UL;
const int TOUCH_DELTA = 150;

int32_t getUVIndexColor(float uvIndex) {
  if (uvIndex >= 0 && uvIndex <= 2) {
    return TFT_GREEN;
  } else if (uvIndex >= 3 && uvIndex <= 5) {
    return TFT_GOLD;
  } else if (uvIndex >= 6 && uvIndex <= 7) {
    return TFT_ORANGE;
  } else if (uvIndex >= 8 && uvIndex <= 10) {
    return TFT_RED;
  } else if (uvIndex >= 11) {
    return TFT_VIOLET;
  }
  return TFT_BLUE;
}

void drawUVIndexBar(float uvIndex, int32_t x, int32_t y) {
    int barHeight = 220;
    int barWidth = 40;
    int fullUVIndexBarValue = 11;
    tft.drawRect(x, y, barWidth, barHeight, TFT_BLACK);
    int fillHeight = min((int) ceil((barHeight / fullUVIndexBarValue) * uvIndex), barHeight - 2);
    tft.fillRect(x + 1, (y + barHeight - fillHeight - 1), barWidth - 2, fillHeight, getUVIndexColor(uvIndex));
}

void drawImageWithoutWhite(int16_t startX, int16_t startY, int16_t width, int16_t h, const uint16_t *img, uint16_t whiteValue = 0xFFFF) {
  for (int16_t y = 0; y < h; y++) {
    int32_t row = (int32_t)y * width;
    for (int16_t x = 0; x < width; x++) {
      uint16_t pixel = img[row + x];
      if (pixel == whiteValue) {
        continue;
      }
      tft.drawPixel(startX + x, startY + y, pixel);
    }
  }
}

int touchStartMs = 0;
int lastTouchMs = 0;

bool checkForExtendedTouch() {
  int touchValue = touchRead(TC_PIN);
  bool beingTouched = touchValue > 1900 || touchValue < 1575;
  if (beingTouched) {
    lastTouchMs = millis();
    if (touchStartMs == 0) {
      touchStartMs = lastTouchMs;
    }
    if (DEBUG) {
      tft.drawCentreString("Being touched " + String(touchValue), 75, 0, 2);
    }
    unsigned long msSinceTouchStart = millis() - touchStartMs;
    if (msSinceTouchStart >= EXTENDED_TOUCH_DURATION_IN_MS) {
      touchStartMs = 0;
      return true;
    }
  } else {
    if (DEBUG) {
      tft.drawCentreString("Not being touched " + String(touchValue), 75, 0, 2);
    }
    int msSinceLastTouch = millis() - lastTouchMs;
    if (msSinceLastTouch > 1000) {
      touchStartMs = 0;
    }
  }
  return false;
}

void drawUVDisplay() {
  tft.fillScreen(BACKGROUND_COLOR);
  String errorFetchingUV = "Unknown error";
  float currentUVIndex = fetchCurrentUVIndex(config.latitude, config.longitude, "Auto", errorFetchingUV);
  if (!isfinite(currentUVIndex)) {
    tft.drawCentreString("Fetching UV index failed:", 120, 80, 2);
    tft.drawCentreString(errorFetchingUV, 120, 100, 2);
    return;
  }
  tft.drawCentreString(String(currentUVIndex, 1), 95, 180, 6);
  drawUVIndexBar(currentUVIndex, 190, 10);
  drawImageWithoutWhite(20, 10, SUN_WIDTH, SUN_HEIGHT, sun);
}

void setup() {
  Serial.begin(115200);

  pinMode(TFT_PWER_PIN, OUTPUT);
  digitalWrite(TFT_PWER_PIN, LOW);

  tft.begin();
  tft.setSwapBytes(true);
  tft.setRotation(0);
  
  config = loadConfig();
  bool bootToConfig = config.bootToConfig;
  config.bootToConfig = false;
  saveConfig(config);

  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextColor(TFT_BLACK);

  if (DEBUG_ALWAYS_START_PORTAL || bootToConfig || !isValidConfig(config)) {
    currentState = CONFIGURATION_MODE;
    return;
  }

  currentState = CONNECT_WIFI;
}

void rebootToConfig() {
  config.bootToConfig = true;
  saveConfig(config);
  esp_restart();
}

void handleConfigurationMode() {
  tft.fillScreen(BACKGROUND_COLOR);
  const String configSsid = "UV-TV-Config";
  const String configPassword = "sunscreen";
  const IPAddress configIp = IPAddress(10, 7, 19, 98);
  tft.drawCentreString("Configuration Mode", 120, 0, 4);
  tft.drawCentreString("Connect to WiFi:", 120, 35, 4);
  tft.drawCentreString(configSsid, 120, 70, 4);
  tft.drawCentreString("using password:", 120, 105, 4);
  tft.drawCentreString(configPassword, 120, 140, 4);
  tft.drawCentreString("then navigate to:", 120, 175, 4);
  tft.drawCentreString("http://" + configIp.toString(), 120, 210, 4);
  broadcastWiFi(configSsid, configPassword, configIp, IPAddress(255, 255, 255, 0));
  config = hostConfigurationServer(config);
  saveConfig(config);
  esp_restart();
}

void handleConnectWiFi() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.drawCentreString("Connecting to", 100, 80, 2);
  tft.drawCentreString(config.wifiSsid, 100, 110, 2);
  if (DEBUG) {
    tft.drawCentreString(config.wifiPassword, 120, 140, 2);
  }
  bool wifiConnected = connectWiFi(config.wifiSsid, config.wifiPassword);
  if (wifiConnected) {
    if (DEBUG) {
      tft.drawCentreString("Connected.", 120, 100, 2);
    }
    currentState = DISPLAY_UV;
  } else {
    tft.drawCentreString("Failed to connect to WiFi", 120, 160, 2);
    delay(5000);
    rebootToConfig();
  }
}

void handleDisplayUV() {
  unsigned long msSinceLastUVFetch = millis() - lastUVFetchMs;
  if (lastUVFetchMs == -1 || msSinceLastUVFetch >= UV_UPDATE_INTERVAL_IN_MS) {
    lastUVFetchMs = millis();
    drawUVDisplay();
  }
}

void loop() {
  // probably good to start fresh every now and then...
  if (millis() > RESTART_INTERVAL) {
    esp_restart();
  }
  if (DEBUG) {
    tft.fillRect(0, 0, 175, 80, BACKGROUND_COLOR);
  }
  bool extendedTouchDetected = checkForExtendedTouch();
  if (DEBUG) {
    tft.drawCentreString(extendedTouchDetected ? "Extended touch" : "No extended touch", 75, 25, 2);
  }
  if (extendedTouchDetected) {
    rebootToConfig();
  }
  if (currentState == DISPLAY_UV) {
    handleDisplayUV();
  } else if (currentState == CONFIGURATION_MODE) {
    handleConfigurationMode();
  } else if (currentState == CONNECT_WIFI) {
    handleConnectWiFi();
  }
}
