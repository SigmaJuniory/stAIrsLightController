#include "config_parser.h"
#include "fakes/fake_pwm_device.h"
#include "staircase.h"
#include "static_map.h"

#include <ctime>
#include <gtest/gtest.h>
#include <stdexcept>

namespace {

FakePWMDevice fakePWM_1(16);
FakePWMDevice fakePWM_2(16);
FakePWMDevice fakePWM_3(16);

static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> fakeDevices = {
    {BASE_I2C_ADRESS, &fakePWM_1},
    {BASE_I2C_ADRESS + 1, &fakePWM_2},
    {BASE_I2C_ADRESS + 2, &fakePWM_3}};

constexpr BrightnessPercentage defaultDayYellowBrightnessPercentage = 80;
constexpr BrightnessPercentage defaultDayWhiteBrightnessPercentage = 60;
constexpr BrightnessPercentage defaultNightYellowBrightnessPercentage = 10;
constexpr BrightnessPercentage defaultNightWhiteBrightnessPercentage = 5;

constexpr BrightnessPercentage stairSpecyficDayYellowBrightnessPercentage = 50;
constexpr BrightnessPercentage stairSpecyficDayWhiteBrightnessPercentage = 40;
constexpr BrightnessPercentage stairSpecyficNightYellowBrightnessPercentage = 10;
constexpr BrightnessPercentage stairSpecyficNightWhiteBrightnessPercentage = 15;

// Expected PWM values are literal conversions from percentage inputs, using floor(4095 * percent /
// 100).
constexpr PwmValue expectedDefaultDayYellowPwm = 3276;
constexpr PwmValue expectedDefaultDayWhitePwm = 2457;
constexpr PwmValue expectedDefaultNightYellowPwm = 409;
constexpr PwmValue expectedDefaultNightWhitePwm = 204;

constexpr PwmValue expectedStairSpecyficDayYellowPwm = 2047;
constexpr PwmValue expectedStairSpecyficDayWhitePwm = 1638;
constexpr PwmValue expectedStairSpecyficNightYellowPwm = 409;
constexpr PwmValue expectedStairSpecyficNightWhitePwm = 614;

constexpr uint8_t numberOfStairsWithoutLightMode = 5;
constexpr uint8_t numberOfStairsWithLightMode = 18;

constexpr Config createTestConfig() {
    Config cfg;

    cfg.globalSettings.lightMode =
        LightMode{.day = {.yellowLightBrightness = defaultDayYellowBrightnessPercentage,
                          .whiteLightBrightness = defaultDayWhiteBrightnessPercentage},
                  .night = {.yellowLightBrightness = defaultNightYellowBrightnessPercentage,
                            .whiteLightBrightness = defaultNightWhiteBrightnessPercentage}};

    StairConfig stairWithoutLightMode{
        .stepsCount = numberOfStairsWithoutLightMode, .hasLightMode = false, .lightMode = {}};

    StairConfig stairWithLightMode{
        .stepsCount = numberOfStairsWithLightMode,
        .hasLightMode = true,
        .lightMode = {
            .day = {.yellowLightBrightness = stairSpecyficDayYellowBrightnessPercentage,
                    .whiteLightBrightness = stairSpecyficDayWhiteBrightnessPercentage},
            .night = {.yellowLightBrightness = stairSpecyficNightYellowBrightnessPercentage,
                      .whiteLightBrightness = stairSpecyficNightWhiteBrightnessPercentage}}};

    cfg.stairs.push_back(stairWithoutLightMode);
    cfg.stairs.push_back(stairWithLightMode);

    return cfg;
}

constexpr Config config = createTestConfig();

constexpr Config createConfigWithStepsCount(const int stepsCount) {
    Config cfg;
    cfg.globalSettings.lightMode =
        LightMode{.day = {.yellowLightBrightness = defaultDayYellowBrightnessPercentage,
                          .whiteLightBrightness = defaultDayWhiteBrightnessPercentage},
                  .night = {.yellowLightBrightness = defaultNightYellowBrightnessPercentage,
                            .whiteLightBrightness = defaultNightWhiteBrightnessPercentage}};
    cfg.stairs.push_back(
        StairConfig{.stepsCount = stepsCount, .hasLightMode = false, .lightMode = {}});
    return cfg;
}

const auto staircasesCount = StaircaseFactory::countConfiguredSteps(config);

} // namespace

class StaircaseFactoryTest : public ::testing::Test {
  protected:
    static constexpr uint8_t getYellowChannel(size_t stepIndex) {
        return (stepIndex % STEPS_PER_PWM_DEVICE) * 2;
    }

    static constexpr uint8_t getWhiteChannel(size_t stepIndex) {
        return (stepIndex % STEPS_PER_PWM_DEVICE) * 2 + 1;
    }
};

TEST_F(StaircaseFactoryTest, countsRequiredPwmExpandersFromConfiguredSteps) {
    EXPECT_EQ(StaircaseFactory::countRequiredPwmExpanders(config), 3u);
}

TEST_F(StaircaseFactoryTest, countsExpectedNumberOfPwmExpandersForSampleStepCounts) {
    EXPECT_EQ(StaircaseFactory::countRequiredPwmExpanders(createConfigWithStepsCount(32)), 4u);
    EXPECT_EQ(StaircaseFactory::countRequiredPwmExpanders(createConfigWithStepsCount(40)), 5u);
    EXPECT_EQ(StaircaseFactory::countRequiredPwmExpanders(createConfigWithStepsCount(48)), 6u);
}

TEST_F(StaircaseFactoryTest, assignsStepsAcrossFourExpandersForThirtyTwoSteps) {
    Config cfg = createConfigWithStepsCount(32);

    FakePWMDevice pwm0(16);
    FakePWMDevice pwm1(16);
    FakePWMDevice pwm2(16);
    FakePWMDevice pwm3(16);

    static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> pwmDevices;
    pwmDevices.insert(BASE_I2C_ADRESS + 0, &pwm0);
    pwmDevices.insert(BASE_I2C_ADRESS + 1, &pwm1);
    pwmDevices.insert(BASE_I2C_ADRESS + 2, &pwm2);
    pwmDevices.insert(BASE_I2C_ADRESS + 3, &pwm3);

    auto staircases = StaircaseFactory::createStaircaseFromConfig(cfg, pwmDevices);

    ASSERT_EQ(staircases.size(), 32u);
    EXPECT_EQ(staircases[0].getStepMapping().expanderI2CAddress, BASE_I2C_ADRESS + 0);
    EXPECT_EQ(staircases[7].getStepMapping().expanderI2CAddress, BASE_I2C_ADRESS + 0);
    EXPECT_EQ(staircases[8].getStepMapping().expanderI2CAddress, BASE_I2C_ADRESS + 1);
    EXPECT_EQ(staircases[15].getStepMapping().expanderI2CAddress, BASE_I2C_ADRESS + 1);
    EXPECT_EQ(staircases[16].getStepMapping().expanderI2CAddress, BASE_I2C_ADRESS + 2);
    EXPECT_EQ(staircases[23].getStepMapping().expanderI2CAddress, BASE_I2C_ADRESS + 2);
    EXPECT_EQ(staircases[24].getStepMapping().expanderI2CAddress, BASE_I2C_ADRESS + 3);
    EXPECT_EQ(staircases[31].getStepMapping().expanderI2CAddress, BASE_I2C_ADRESS + 3);
}

TEST_F(StaircaseFactoryTest, rejectsConfigurationsThatExceedSupportedStepCapacity) {
    Config oversizedConfig;
    oversizedConfig.globalSettings.lightMode = config.globalSettings.lightMode;
    oversizedConfig.stairs.push_back(
        StairConfig{.stepsCount = MAX_NUM_OF_STEPS + 1, .hasLightMode = false, .lightMode = {}});

    EXPECT_THROW(StaircaseFactory::validateConfig(oversizedConfig), std::invalid_argument);
}

TEST_F(StaircaseFactoryTest, verifyStepMapping) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);

    ASSERT_EQ(staircases.size(), staircasesCount);

    // Verify modulo logic
    for (size_t i = 0; i < staircasesCount; i++) {
        uint8_t expanderIndex = i / STEPS_PER_PWM_DEVICE;
        EXPECT_EQ(staircases[i].getStepMapping().expanderI2CAddress,
                  BASE_I2C_ADRESS + expanderIndex);
    }

    // Verify brightness values are set correctly
    for (size_t i = 0; i < numberOfStairsWithoutLightMode; i++) {
        const auto stepMapping = staircases[i].getStepMapping();

        EXPECT_EQ(stepMapping.dayYellowBrightness, defaultDayYellowBrightnessPercentage);
        EXPECT_EQ(stepMapping.dayWhiteBrightness, defaultDayWhiteBrightnessPercentage);
        EXPECT_EQ(stepMapping.nightYellowBrightness, defaultNightYellowBrightnessPercentage);
        EXPECT_EQ(stepMapping.nightWhiteBrightness, defaultNightWhiteBrightnessPercentage);
    }

    for (size_t i = numberOfStairsWithoutLightMode; i < staircasesCount; i++) {
        const auto stepMapping = staircases[i].getStepMapping();

        EXPECT_EQ(stepMapping.dayYellowBrightness, stairSpecyficDayYellowBrightnessPercentage);
        EXPECT_EQ(stepMapping.dayWhiteBrightness, stairSpecyficDayWhiteBrightnessPercentage);
        EXPECT_EQ(stepMapping.nightYellowBrightness, stairSpecyficNightYellowBrightnessPercentage);
        EXPECT_EQ(stepMapping.nightWhiteBrightness, stairSpecyficNightWhiteBrightnessPercentage);
    }
}

TEST_F(StaircaseFactoryTest, DayAndNightModeBrightnessForGlobalSettings) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);
    constexpr size_t stepIndex = 0;
    constexpr uint8_t yellowChan = getYellowChannel(stepIndex);
    constexpr uint8_t whiteChan = getWhiteChannel(stepIndex);

    staircases[stepIndex].setMode(LightModeE::DayMode);
    staircases[stepIndex].setYellow();
    staircases[stepIndex].setWhite();
    EXPECT_EQ(fakePWM_1.getPWM(yellowChan), expectedDefaultDayYellowPwm);
    EXPECT_EQ(fakePWM_1.getPWM(whiteChan), expectedDefaultDayWhitePwm);

    staircases[stepIndex].setMode(LightModeE::NightMode);
    staircases[stepIndex].setYellow();
    staircases[stepIndex].setWhite();
    EXPECT_EQ(fakePWM_1.getPWM(yellowChan), expectedDefaultNightYellowPwm);
    EXPECT_EQ(fakePWM_1.getPWM(whiteChan), expectedDefaultNightWhitePwm);
}

TEST_F(StaircaseFactoryTest, SetAllMethodAppliesStairSpecificBrightness) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);
    constexpr size_t stepIndex = 5;
    constexpr uint8_t yellowChan = getYellowChannel(stepIndex);
    constexpr uint8_t whiteChan = getWhiteChannel(stepIndex);

    staircases[stepIndex].setMode(LightModeE::DayMode);
    staircases[stepIndex].setAll();
    EXPECT_EQ(fakePWM_1.getPWM(yellowChan), expectedStairSpecyficDayYellowPwm);
    EXPECT_EQ(fakePWM_1.getPWM(whiteChan), expectedStairSpecyficDayWhitePwm);

    staircases[stepIndex].setMode(LightModeE::NightMode);
    staircases[stepIndex].setAll();
    EXPECT_EQ(fakePWM_1.getPWM(yellowChan), expectedStairSpecyficNightYellowPwm);
    EXPECT_EQ(fakePWM_1.getPWM(whiteChan), expectedStairSpecyficNightWhitePwm);
}

TEST_F(StaircaseFactoryTest, CorrectlyRoutesPWMToSecondExpander) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);
    constexpr size_t stepIndex = 8;
    constexpr uint8_t yellowChan = getYellowChannel(stepIndex);

    staircases[stepIndex].setMode(LightModeE::DayMode);
    staircases[stepIndex].setYellow();
    EXPECT_EQ(fakePWM_2.getPWM(yellowChan), expectedStairSpecyficDayYellowPwm);

    staircases[stepIndex].setMode(LightModeE::NightMode);
    staircases[stepIndex].setYellow();
    EXPECT_EQ(fakePWM_2.getPWM(yellowChan), expectedStairSpecyficNightYellowPwm);
}

TEST_F(StaircaseFactoryTest, LegacySetWarmMethodWorksCorrectly) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);
    constexpr size_t stepIndex = 16;
    constexpr uint8_t yellowChan = getYellowChannel(stepIndex);

    staircases[stepIndex].setMode(LightModeE::DayMode);
    staircases[stepIndex].setWarm();
    EXPECT_EQ(fakePWM_3.getPWM(yellowChan), expectedStairSpecyficDayYellowPwm);

    staircases[stepIndex].setMode(LightModeE::NightMode);
    staircases[stepIndex].setWarm();
    EXPECT_EQ(fakePWM_3.getPWM(yellowChan), expectedStairSpecyficNightYellowPwm);
}

TEST_F(StaircaseFactoryTest, AutomaticTimeBasedModeDetection) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);

    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    int currentHour = timeinfo->tm_hour;

    bool shouldBeDay = (currentHour >= DAY_START && currentHour < NIGHT_START);

    staircases[0].updateModeBasedOnTime();

    if (shouldBeDay) {
        staircases[0].setAll();
        EXPECT_EQ(fakePWM_1.getPWM(0), expectedDefaultDayYellowPwm);
        EXPECT_EQ(fakePWM_1.getPWM(1), expectedDefaultDayWhitePwm);
    } else {
        staircases[0].setAll();
        EXPECT_EQ(fakePWM_1.getPWM(0), expectedDefaultNightYellowPwm);
        EXPECT_EQ(fakePWM_1.getPWM(1), expectedDefaultNightWhitePwm);
    }
}
