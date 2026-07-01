#include "config_parser.h"
#include "fakes/fake_pwm_device.h"
#include "staircase.h"
#include "static_map.h"

#include <ctime>
#include <gtest/gtest.h>

namespace {

FakePWMDevice fakePWM_1(16);
FakePWMDevice fakePWM_2(16);
FakePWMDevice fakePWM_3(16);

static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> fakeDevices = {
    {BASE_I2C_ADRESS, &fakePWM_1},
    {BASE_I2C_ADRESS + 1, &fakePWM_2},
    {BASE_I2C_ADRESS + 2, &fakePWM_3}};

constexpr PwmValue defaultDayYellowBrightness = 2000;
constexpr PwmValue defaultDayWhiteBrightness = 1800;
constexpr PwmValue defaultNightYellowBrightness = 100;
constexpr PwmValue defaultNightWhiteBrightness = 100;

constexpr PwmValue stairSpecyficDayYellowBrightness = 1500;
constexpr PwmValue stairSpecyficDayWhiteBrightness = 1200;
constexpr PwmValue stairSpecyficNightYellowBrightness = 100;
constexpr PwmValue stairSpecyficNightWhiteBrightness = 150;

constexpr uint8_t numberOfStairsWithoutLightMode = 5;
constexpr uint8_t numberOfStairsWithLightMode = 18;

constexpr Config createTestConfig() {
    Config cfg;

    cfg.globalSettings.lightMode =
        LightMode{.day = {.yellowLightBrightness = defaultDayYellowBrightness,
                          .whiteLightBrightness = defaultDayWhiteBrightness},
                  .night = {.yellowLightBrightness = defaultNightYellowBrightness,
                            .whiteLightBrightness = defaultNightWhiteBrightness}};

    StairConfig stairWithoutLightMode{
        .stepsCount = numberOfStairsWithoutLightMode, .hasLightMode = false, .lightMode = {}};

    StairConfig stairWithLightMode{
        .stepsCount = numberOfStairsWithLightMode,
        .hasLightMode = true,
        .lightMode = {.day = {.yellowLightBrightness = stairSpecyficDayYellowBrightness,
                              .whiteLightBrightness = stairSpecyficDayWhiteBrightness},
                      .night = {.yellowLightBrightness = stairSpecyficNightYellowBrightness,
                                .whiteLightBrightness = stairSpecyficNightWhiteBrightness}}};

    cfg.stairs.push_back(stairWithoutLightMode);
    cfg.stairs.push_back(stairWithLightMode);

    return cfg;
}

constexpr Config config = createTestConfig();

const auto staircasesCount = []() {
    uint8_t count = 0;
    for (const auto &stairConfig : config.stairs) {
        count += stairConfig.stepsCount;
    }
    return count;
}();

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

        EXPECT_EQ(stepMapping.dayYellowBrightness, defaultDayYellowBrightness);
        EXPECT_EQ(stepMapping.dayWhiteBrightness, defaultDayWhiteBrightness);
        EXPECT_EQ(stepMapping.nightYellowBrightness, defaultNightYellowBrightness);
        EXPECT_EQ(stepMapping.nightWhiteBrightness, defaultNightWhiteBrightness);
    }

    for (size_t i = numberOfStairsWithoutLightMode; i < staircasesCount; i++) {
        const auto stepMapping = staircases[i].getStepMapping();

        EXPECT_EQ(stepMapping.dayYellowBrightness, stairSpecyficDayYellowBrightness);
        EXPECT_EQ(stepMapping.dayWhiteBrightness, stairSpecyficDayWhiteBrightness);
        EXPECT_EQ(stepMapping.nightYellowBrightness, stairSpecyficNightYellowBrightness);
        EXPECT_EQ(stepMapping.nightWhiteBrightness, stairSpecyficNightWhiteBrightness);
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
    EXPECT_EQ(fakePWM_1.getPWM(yellowChan), defaultDayYellowBrightness);
    EXPECT_EQ(fakePWM_1.getPWM(whiteChan), defaultDayWhiteBrightness);

    staircases[stepIndex].setMode(LightModeE::NightMode);
    staircases[stepIndex].setYellow();
    staircases[stepIndex].setWhite();
    EXPECT_EQ(fakePWM_1.getPWM(yellowChan), defaultNightYellowBrightness);
    EXPECT_EQ(fakePWM_1.getPWM(whiteChan), defaultNightWhiteBrightness);
}

TEST_F(StaircaseFactoryTest, SetAllMethodAppliesStairSpecificBrightness) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);
    constexpr size_t stepIndex = 5;
    constexpr uint8_t yellowChan = getYellowChannel(stepIndex);
    constexpr uint8_t whiteChan = getWhiteChannel(stepIndex);

    staircases[stepIndex].setMode(LightModeE::DayMode);
    staircases[stepIndex].setAll();
    EXPECT_EQ(fakePWM_1.getPWM(yellowChan), stairSpecyficDayYellowBrightness);
    EXPECT_EQ(fakePWM_1.getPWM(whiteChan), stairSpecyficDayWhiteBrightness);

    staircases[stepIndex].setMode(LightModeE::NightMode);
    staircases[stepIndex].setAll();
    EXPECT_EQ(fakePWM_1.getPWM(yellowChan), stairSpecyficNightYellowBrightness);
    EXPECT_EQ(fakePWM_1.getPWM(whiteChan), stairSpecyficNightWhiteBrightness);
}

TEST_F(StaircaseFactoryTest, CorrectlyRoutesPWMToSecondExpander) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);
    constexpr size_t stepIndex = 8;
    constexpr uint8_t yellowChan = getYellowChannel(stepIndex);

    staircases[stepIndex].setMode(LightModeE::DayMode);
    staircases[stepIndex].setYellow();
    EXPECT_EQ(fakePWM_2.getPWM(yellowChan), stairSpecyficDayYellowBrightness);

    staircases[stepIndex].setMode(LightModeE::NightMode);
    staircases[stepIndex].setYellow();
    EXPECT_EQ(fakePWM_2.getPWM(yellowChan), stairSpecyficNightYellowBrightness);
}

TEST_F(StaircaseFactoryTest, LegacySetWarmMethodWorksCorrectly) {
    auto staircases = StaircaseFactory::createStaircaseFromConfig(config, fakeDevices);
    constexpr size_t stepIndex = 16;
    constexpr uint8_t yellowChan = getYellowChannel(stepIndex);

    staircases[stepIndex].setMode(LightModeE::DayMode);
    staircases[stepIndex].setWarm();
    EXPECT_EQ(fakePWM_3.getPWM(yellowChan), stairSpecyficDayYellowBrightness);

    staircases[stepIndex].setMode(LightModeE::NightMode);
    staircases[stepIndex].setWarm();
    EXPECT_EQ(fakePWM_3.getPWM(yellowChan), stairSpecyficNightYellowBrightness);
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
        EXPECT_EQ(fakePWM_1.getPWM(0), defaultDayYellowBrightness);
        EXPECT_EQ(fakePWM_1.getPWM(1), defaultDayWhiteBrightness);
    } else {
        staircases[0].setAll();
        EXPECT_EQ(fakePWM_1.getPWM(0), defaultNightYellowBrightness);
        EXPECT_EQ(fakePWM_1.getPWM(1), defaultNightWhiteBrightness);
    }
}
