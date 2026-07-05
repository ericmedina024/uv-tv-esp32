struct Config {
  String wifiSsid = "";
  String wifiPassword = "";
  double longitude = 0;
  double latitude = 0;
  bool bootToConfig = false;
};

bool isValidConfig(const Config& config);

Config loadConfig();

void saveConfig(const Config &config);
