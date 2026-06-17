#include "ConfigParser.h"
#include "StaticVector.h"
#include "fakes/FakePWMDevice.h"
#include <ArduinoJson.h>
#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include <map>

class StaircaseFactoryTest : public ::testing::Test {};

struct StepMapping {
    uint8_t stepId;
    uint8_t stairIndex;
    uint8_t expanderI2CAddress;
    uint8_t dayYellowBrightness;
    uint8_t dayWhiteBrightness;
    uint8_t nightYellowBrightness;
    uint8_t nightWhiteBrightness;
};

// static_vector<StepMapping, 64> stepMappings; // Max 64 steps (8 expanders * 8 steps)

class StairStep {
  public:
    StairStep() = default; // needed for creating static_vector;
    explicit StairStep(const StepMapping &stepMapping, IPWMDevice *pwmDevice)
        : stepMapping(stepMapping), pwmDevice(pwmDevice), isDayMode(true) {}

    // Auto-detect day/night mode based on current time (7-19 = day, else = night)
    void updateModeBasedOnTime() {
        time_t now = time(nullptr);
        struct tm *timeinfo = localtime(&now);
        int hour = timeinfo->tm_hour;
        isDayMode = (hour >= 7 && hour < 19);
    }

    void setYellow() {
        uint8_t brightness =
            isDayMode ? stepMapping.dayYellowBrightness : stepMapping.nightYellowBrightness;
        uint8_t channelWarm = (stepMapping.stepId % 8) * 2;
        pwmDevice->setPWM(channelWarm, brightness);
    }

    void setWhite() {
        uint8_t brightness =
            isDayMode ? stepMapping.dayWhiteBrightness : stepMapping.nightWhiteBrightness;
        uint8_t channelCold = (stepMapping.stepId % 8) * 2 + 1;
        pwmDevice->setPWM(channelCold, brightness);
    }

    void setAll() {
        setYellow();
        setWhite();
    }

    void setMode(bool dayMode) {
        isDayMode = dayMode;
    }

    bool getMode() const {
        return isDayMode;
    }

    // Legacy method name for compatibility
    void setWarm() {
        setYellow();
    }

    StepMapping stepMapping; // it supose to be private
  private:
    IPWMDevice *pwmDevice;
    bool isDayMode;
};

class StaircaseFactory {
  public:
    static static_vector<StairStep, 64>
    createStaircaseFromConfig(const Config &config, std::map<uint8_t, IPWMDevice *> &pwmDevices) {
        static_vector<StairStep, 64> staircases;

        uint8_t globalStepId = 0;
        uint8_t baseI2CAddress = 0x40;

        for (size_t stairIdx = 0; stairIdx < config.stairs.size(); stairIdx++) {
            const auto &stairConfig = config.stairs[stairIdx];
            for (int i = 0; i < stairConfig.stepsCount; i++) {
                uint8_t expanderIndex = globalStepId / 8;
                uint8_t expanderI2CAddress = baseI2CAddress + expanderIndex;

                LightMode lightMode = (stairConfig.hasLightMode) ? stairConfig.lightMode
                                                                 : config.globalSettings.lightMode;

                StepMapping stepMapping{
                    .stepId = globalStepId,
                    .stairIndex = static_cast<uint8_t>(stairIdx),
                    .expanderI2CAddress = expanderI2CAddress,
                    .dayYellowBrightness = lightMode.day.yellowLightBrightness,
                    .dayWhiteBrightness = lightMode.day.whiteLightBrightness,
                    .nightYellowBrightness = lightMode.night.yellowLightBrightness,
                    .nightWhiteBrightness = lightMode.night.whiteLightBrightness};

                staircases.push_back(StairStep(stepMapping, pwmDevices[expanderI2CAddress]));

                globalStepId++;
            }
        }
        return staircases;
    }
};

namespace {
FakePWMDevice fakePWM_0x40(16);
FakePWMDevice fakePWM_0x41(16);
FakePWMDevice fakePWM_0x42(16);

std::map<uint8_t, IPWMDevice *> fakeDevices = {
    {0x40, &fakePWM_0x40}, {0x41, &fakePWM_0x41}, {0x42, &fakePWM_0x42}};

std::string configPath = "test/test_native/config.json";
const Config config = ConfigParser::parseConfigFromFile(configPath);

} // namespace

TEST_F(StaircaseFactoryTest, verifyStepMapping) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);

    // Verify step mappings (5 + 18 = 23 total steps)
    ASSERT_EQ(staircases.size(), 23);

    // Verify modulo 8 logic: steps 0-7 use 0x40, steps 8-15 use 0x41, steps 16-23 use 0x42
    // Verify brightness values are set correctly
    for (size_t i = 0; i < 5; i++) {
        EXPECT_EQ(staircases[i].stepMapping.expanderI2CAddress, 0x40);
        EXPECT_EQ(staircases[i].stepMapping.dayYellowBrightness, 80);    // global default day
        EXPECT_EQ(staircases[i].stepMapping.dayWhiteBrightness, 60);     // global default day
        EXPECT_EQ(staircases[i].stepMapping.nightYellowBrightness, 100); // global default night
        EXPECT_EQ(staircases[i].stepMapping.nightWhiteBrightness, 100);  // global default night
    }

    for (size_t i = 5; i < 23; i++) {
        uint8_t expanderIndex = i / 8;
        EXPECT_EQ(staircases[i].stepMapping.expanderI2CAddress, 0x40 + expanderIndex);
        EXPECT_EQ(staircases[i].stepMapping.dayYellowBrightness, 50);    // stair-specific day
        EXPECT_EQ(staircases[i].stepMapping.dayWhiteBrightness, 40);     // stair-specific day
        EXPECT_EQ(staircases[i].stepMapping.nightYellowBrightness, 100); // stair-specific night
        EXPECT_EQ(staircases[i].stepMapping.nightWhiteBrightness, 100);  // stair-specific night
    }
}

TEST_F(StaircaseFactoryTest, checkLightModes) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);

    // Verify all steps were created (5 + 18 = 23)
    ASSERT_EQ(staircases.size(), 23);

    // Test DAY MODE for first stair (steps 0-4, yellow: 80, white: 60)
    staircases[0].setMode(true); // day mode
    staircases[0].setYellow();
    staircases[0].setWhite();

    EXPECT_EQ(fakePWM_0x40.getPWM(0), 80); // step 0, channel yellow, day brightness
    EXPECT_EQ(fakePWM_0x40.getPWM(1), 60); // step 0, channel white, day brightness

    // Test NIGHT MODE for first stair (steps 0-4, yellow: 100, white: 100)
    staircases[0].setMode(false); // night mode
    staircases[0].setYellow();
    staircases[0].setWhite();

    EXPECT_EQ(fakePWM_0x40.getPWM(0), 100); // step 0, channel yellow, night brightness
    EXPECT_EQ(fakePWM_0x40.getPWM(1), 100); // step 0, channel white, night brightness

    // Test setAll() in day mode for second stair step (yellow: 50, white: 40)
    staircases[5].setMode(true); // day mode
    staircases[5].setAll();

    EXPECT_EQ(fakePWM_0x40.getPWM(10), 50); // step 5, channel yellow, day mode
    EXPECT_EQ(fakePWM_0x40.getPWM(11), 40); // step 5, channel white, day mode

    // Test setAll() in night mode for second stair (yellow: 100, white: 100)
    staircases[5].setMode(false); // night mode
    staircases[5].setAll();

    EXPECT_EQ(fakePWM_0x40.getPWM(10), 100); // step 5, channel yellow, night mode
    EXPECT_EQ(fakePWM_0x40.getPWM(11), 100); // step 5, channel white, night mode

    // Test across expanders (step 8, stair 2, yellow: 50 day / 100 night, white: 40 day / 100
    // night)
    staircases[8].setMode(true);
    staircases[8].setYellow();
    EXPECT_EQ(fakePWM_0x41.getPWM(0), 50); // day mode

    staircases[8].setMode(false);
    staircases[8].setYellow();
    EXPECT_EQ(fakePWM_0x41.getPWM(0), 100); // night mode

    // Test legacy setWarm() with day mode
    staircases[16].setMode(true);
    staircases[16].setWarm();
    EXPECT_EQ(fakePWM_0x42.getPWM(0), 50); // step 16, day mode

    // Test legacy setWarm() with night mode
    staircases[16].setMode(false);
    staircases[16].setWarm();
    EXPECT_EQ(fakePWM_0x42.getPWM(0), 100); // step 16, night mode
}

TEST_F(StaircaseFactoryTest, AutomaticTimeBasedModeDetection) {

    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);

    // Test automatic mode detection based on current time
    // Hours 7-19 should be day mode, others should be night mode
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    int currentHour = timeinfo->tm_hour;
    bool shouldBeDay = (currentHour >= 7 && currentHour < 19);

    staircases[0].updateModeBasedOnTime();

    if (shouldBeDay) {
        // In day mode (7-19): yellow 80, white 60
        staircases[0].setAll();
        EXPECT_EQ(fakePWM_0x40.getPWM(0), 80); // yellow day
        EXPECT_EQ(fakePWM_0x40.getPWM(1), 60); // white day
    } else {
        // In night mode (<7 or >=19): yellow 100, white 100
        staircases[0].setAll();
        EXPECT_EQ(fakePWM_0x40.getPWM(0), 100); // yellow night
        EXPECT_EQ(fakePWM_0x40.getPWM(1), 100); // white night
    }
}
