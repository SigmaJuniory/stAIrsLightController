#include <ArduinoJson.h>
#include <gtest/gtest.h>

namespace {
constexpr auto jsonInput = R"({
        "sensor": "gps",
        "time": 1351824120
    })";

constexpr auto jsonConfig = R"({
        "stepMappings": [
            {
                "stepId": 1,
                "expanderI2CAddress": 64,
                "channelWarm": 5,
                "channelCold": 6
            },
            {
                "stepId": 2,
                "expanderI2CAddress": 64,
                "channelWarm": 7,
                "channelCold": 8
            }
        ]
    })";
} // namespace

struct StepMapping {
  uint8_t stepId;
  uint8_t expanderI2CAddress;
  uint8_t channelWarm;
  uint8_t channelCold;
};

struct Config {
  std::vector<StepMapping> stepMappings{};
};

class ConfigParser {
public:
  static Config parseConfigFromJson(const std::string_view json) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
      throw std::runtime_error(error.c_str());
    }

    std::vector<StepMapping> stepMappings;
    for (const auto &stepMappingJson : doc["stepMappings"].as<JsonArray>()) {

      stepMappings.push_back(
          StepMapping{.stepId = stepMappingJson["stepId"],
                      .expanderI2CAddress = stepMappingJson["expanderI2CAddress"],
                      .channelWarm = stepMappingJson["channelWarm"],
                      .channelCold = stepMappingJson["channelCold"]});
    }

    return Config{.stepMappings = std::move(stepMappings)};
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

  ASSERT_EQ(config.stepMappings.size(), 2);
  EXPECT_EQ(config.stepMappings[0].stepId, 1);
  EXPECT_EQ(config.stepMappings[0].expanderI2CAddress, 64);
  EXPECT_EQ(config.stepMappings[0].channelWarm, 5);
  EXPECT_EQ(config.stepMappings[0].channelCold, 6);

  EXPECT_EQ(config.stepMappings[1].stepId, 2);
  EXPECT_EQ(config.stepMappings[1].expanderI2CAddress, 64);
  EXPECT_EQ(config.stepMappings[1].channelWarm, 7);
  EXPECT_EQ(config.stepMappings[1].channelCold, 8);
}
