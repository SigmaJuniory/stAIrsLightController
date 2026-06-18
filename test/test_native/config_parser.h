#include <ArduinoJson.h>
#include <array>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <static_vector.h>
#include <string>

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
    static_vector<StairConfig, 10> stairs;
};

class ConfigParser {
  public:
    static Config parseConfigFromFile(const std::string &filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open config file: " + filePath);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string jsonString = buffer.str();

        return parseConfigFromJson(jsonString);
    }

    static Config parseConfigFromJson(const std::string &json) {
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

            stairConfig.lightMode.day.yellowLightBrightness =
                lightMode["day"]["YellowLightBrightness"];

            stairConfig.lightMode.day.whiteLightBrightness =
                lightMode["day"]["WhiteLightBrightness"];

            stairConfig.lightMode.night.yellowLightBrightness =
                lightMode["night"]["YellowLightBrightness"];

            stairConfig.lightMode.night.whiteLightBrightness =
                lightMode["night"]["WhiteLightBrightness"];
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
