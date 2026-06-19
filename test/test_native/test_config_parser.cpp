#include "config_parser.h"
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

TEST_F(ConfigParserTest, ParsesStaircaseConfigFromFile) {
    Config config = ConfigParser::parseConfigFromFile("test/test_native/config.json");

    ASSERT_EQ(config.globalSettings.lightMode.day.yellowLightBrightness, 80);
}