#include "config_parser.h"

#include <ArduinoJson.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

BrightnessPercentage parseBrightnessPercentage(const JsonVariantConst &brightnessJson) {
    const int brightness = brightnessJson.as<int>();
    if (brightness < 0 || brightness > 100) {
        throw std::runtime_error("Brightness percentage must be in range 0-100");
    }

    return static_cast<BrightnessPercentage>(brightness);
}

void parseStairConfig(const JsonObject &stairJson, StairConfig &stairConfig) {
    stairConfig.stepsCount = stairJson["stepsCount"];
    if (auto lightMode = stairJson["lightMode"]; !lightMode.isNull()) {
        stairConfig.hasLightMode = true;

        stairConfig.lightMode.day.yellowLightBrightness =
            parseBrightnessPercentage(lightMode["day"]["YellowLightBrightness"]);

        stairConfig.lightMode.day.whiteLightBrightness =
            parseBrightnessPercentage(lightMode["day"]["WhiteLightBrightness"]);

        stairConfig.lightMode.night.yellowLightBrightness =
            parseBrightnessPercentage(lightMode["night"]["YellowLightBrightness"]);

        stairConfig.lightMode.night.whiteLightBrightness =
            parseBrightnessPercentage(lightMode["night"]["WhiteLightBrightness"]);
    }
}

void parseGlobalSettings(const JsonObject &globalSettingsJson, Config &config) {
    auto lightMode = globalSettingsJson["lightMode"];

    config.globalSettings.lightMode.day.yellowLightBrightness =
        parseBrightnessPercentage(lightMode["day"]["YellowLightBrightness"]);

    config.globalSettings.lightMode.day.whiteLightBrightness =
        parseBrightnessPercentage(lightMode["day"]["WhiteLightBrightness"]);

    config.globalSettings.lightMode.night.yellowLightBrightness =
        parseBrightnessPercentage(lightMode["night"]["YellowLightBrightness"]);

    config.globalSettings.lightMode.night.whiteLightBrightness =
        parseBrightnessPercentage(lightMode["night"]["WhiteLightBrightness"]);
}

} // namespace

Config ConfigParser::parseConfigFromFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseConfigFromJson(buffer.str());
}

Config ConfigParser::parseConfigFromJson(const std::string &json) {
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
