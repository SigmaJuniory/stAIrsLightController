#pragma once

#include "pwm_types.h"

#include <cstddef>
#include <cstdint>
#include <static_vector.h>
#include <string>

constexpr std::size_t MAX_STAIRS_COUNT = 10;

struct LightLevel {
    BrightnessPercentage yellowLightBrightness{};
    BrightnessPercentage whiteLightBrightness{};
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
    static_vector<StairConfig, MAX_STAIRS_COUNT> stairs;
};

class ConfigParser {
  public:
    static Config parseConfigFromFile(const std::string &filePath);
    static Config parseConfigFromJson(const std::string &json);
};
