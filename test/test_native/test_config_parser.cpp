#include <ArduinoJson.h>
#include <fstream>
#include <gtest/gtest.h>

namespace {
constexpr auto jsonInput = R"({
        "sensor": "gps",
        "time": 1351824120
    })";

constexpr auto jsonConfig = R"({
    "globalSettings": {
        "lightMode": {
            "day": {             
                "YellowLightBrightness": 80, "WhiteLightBrightness": 60 },
            "night": { 
                "YellowLightBrightness": 100, "WhiteLightBrightness": 100 }
        }
    },
    "stairs": [
        {
            "stepsCount": 5
        },
        {
            "stepsCount": 18,
            "lightMode": {
                "day": { "YellowLightBrightness": 50, "WhiteLightBrightness": 40 },
                "night": { "YellowLightBrightness": 100, "WhiteLightBrightness": 100 }
            }
        }
    ]
})";

} // namespace

struct LightLevel {
  uint8_t yellowLightBrightness{};
  uint8_t whiteLightBrightness{};
};

struct LightMode {
  LightLevel day;
  LightLevel night;
};

struct GlobalSettings {
  LightMode lightMode;
};

struct StairConfig {
  int stepsCount{};
  bool hasLightMode{};
  LightMode lightMode;
};

struct Config {
  GlobalSettings globalSettings;
  std::vector<StairConfig> stairs{};
};

class ConfigParser {
public:
  static Config parseConfigFromFile(const std::string &filePath) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
      throw std::runtime_error("Cannot open file: " + filePath);
    }

    std::string jsonString((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    file.close();

    return parseConfigFromJson(jsonString);
  }

  static Config parseConfigFromJson(const std::string_view json) {
    JsonDocument doc;
    const auto error = deserializeJson(doc, json);

    if (error) {
      throw std::runtime_error(error.c_str());
    }
    Config config;

    parseGlobalSettings(doc["globalSettings"], config);

    for (const auto &stair : doc["stairs"].as<JsonArray>()) {
      StairConfig stairConfig;

      parseStairConfig(stair, stairConfig);

      config.stairs.push_back(stairConfig);
    }

    return config;
  }

private:
  static void parseStairConfig(const JsonObject &stairJson, StairConfig &stairConfig) {
    stairConfig.stepsCount = stairJson["stepsCount"];
    if (auto lightMode = stairJson["lightMode"]; !lightMode.isNull()) {
      stairConfig.hasLightMode = true;

      stairConfig.lightMode.day.yellowLightBrightness = lightMode["day"]["YellowLightBrightness"];

      stairConfig.lightMode.day.whiteLightBrightness = lightMode["day"]["WhiteLightBrightness"];

      stairConfig.lightMode.night.yellowLightBrightness =
          lightMode["night"]["YellowLightBrightness"];

      stairConfig.lightMode.night.whiteLightBrightness = lightMode["night"]["WhiteLightBrightness"];
    }
  }

  static void parseGlobalSettings(const JsonObject &globalSettingsJson, Config &config) {
    auto lightMode = globalSettingsJson["lightMode"];

    config.globalSettings.lightMode.day.yellowLightBrightness =
        lightMode["day"]["YellowLightBrightness"];

    config.globalSettings.lightMode.day.whiteLightBrightness =
        lightMode["day"]["WhiteLightBrightness"];

    config.globalSettings.lightMode.night.yellowLightBrightness =
        lightMode["night"]["YellowLightBrightness"];

    config.globalSettings.lightMode.night.whiteLightBrightness =
        lightMode["night"]["WhiteLightBrightness"];
  }
};

TEST(Sanity, ParsesFromJson) {
  JsonDocument doc;
  deserializeJson(doc, jsonInput);

  const auto sensor = doc["sensor"];
  const auto time = doc["time"];

  EXPECT_EQ(sensor, "gps");
  EXPECT_EQ(time, 1351824120);
}

class ConfigParserTest : public ::testing::Test {};

TEST_F(ConfigParserTest, ParsesStaircaseConfig) {
  Config config = ConfigParser::parseConfigFromJson(jsonConfig);

  ASSERT_EQ(config.globalSettings.lightMode.day.yellowLightBrightness, 80);
  ASSERT_EQ(config.globalSettings.lightMode.day.whiteLightBrightness, 60);
  ASSERT_EQ(config.globalSettings.lightMode.night.yellowLightBrightness, 100);
  ASSERT_EQ(config.globalSettings.lightMode.night.whiteLightBrightness, 100);

  ASSERT_EQ(config.stairs.size(), 2);

  ASSERT_EQ(config.stairs[0].stepsCount, 5);
  ASSERT_FALSE(config.stairs[0].hasLightMode);

  ASSERT_EQ(config.stairs[1].stepsCount, 18);
  ASSERT_TRUE(config.stairs[1].hasLightMode);
  ASSERT_EQ(config.stairs[1].lightMode.day.yellowLightBrightness, 50);
  ASSERT_EQ(config.stairs[1].lightMode.day.whiteLightBrightness, 40);
  ASSERT_EQ(config.stairs[1].lightMode.night.yellowLightBrightness, 100);
  ASSERT_EQ(config.stairs[1].lightMode.night.whiteLightBrightness, 100);
}
