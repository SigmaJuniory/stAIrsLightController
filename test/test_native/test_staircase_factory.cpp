#include "config_parser.h"
#include "fakes/fake_pwm_device.h"
#include "static_map.h"
#include "static_vector.h"
#include <ArduinoJson.h>
#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include <map>

// TODO: MOVE TO global_const.h etc
namespace {
constexpr uint8_t CHANNELS_PER_EXPANDER = 16;
constexpr uint8_t BASE_I2C_ADRESS = 0x40;
constexpr uint8_t CHANNELS_PER_STEP = 2;
constexpr uint8_t STEPS_PER_PWM_DEVICE = CHANNELS_PER_EXPANDER / CHANNELS_PER_STEP;
constexpr uint8_t DAY_START = 7;
constexpr uint8_t NIGHT_START = 19;
constexpr uint8_t MAX_NUM_OF_STEPS = 64;
} // namespace

enum class LightModeE : uint8_t { DayMode, NightMode };

struct StepMapping {
    uint8_t stepId;
    uint8_t stairIndex;
    uint8_t expanderI2CAddress;
    uint8_t dayYellowBrightness;
    uint8_t dayWhiteBrightness;
    uint8_t nightYellowBrightness;
    uint8_t nightWhiteBrightness;
};

class StairStep {
  public:
    StairStep() = default; // needed for creating static_vector;
    explicit StairStep(const StepMapping &stepMapping, IPWMDevice *pwmDevice)
        : stepMapping(stepMapping), pwmDevice(pwmDevice), mode(LightModeE::DayMode) {}

    void updateModeBasedOnTime() {
        time_t now = time(nullptr);
        struct tm *timeinfo = localtime(&now);
        int hour = timeinfo->tm_hour;
        mode =
            (hour >= DAY_START && hour < NIGHT_START) ? LightModeE::DayMode : LightModeE::NightMode;
    }

    void setYellow() {
        uint8_t brightness = mode == LightModeE::DayMode ? stepMapping.dayYellowBrightness
                                                         : stepMapping.nightYellowBrightness;
        uint8_t channelWarm = (stepMapping.stepId % STEPS_PER_PWM_DEVICE) * 2;
        pwmDevice->setPWM(channelWarm, brightness);
    }

    void setWhite() {
        uint8_t brightness = mode == LightModeE::DayMode ? stepMapping.dayWhiteBrightness
                                                         : stepMapping.nightWhiteBrightness;
        uint8_t channelCold = (stepMapping.stepId % STEPS_PER_PWM_DEVICE) * 2 + 1;
        pwmDevice->setPWM(channelCold, brightness);
    }

    void setAll() {
        setYellow();
        setWhite();
    }

    void setMode(LightModeE mode) {
        this->mode = mode;
    }

    LightModeE getMode() const {
        return mode;
    }

    // Legacy method name for compatibility
    void setWarm() {
        setYellow();
    }

    StepMapping getStepMapping() {
        return stepMapping;
    }

  private:
    StepMapping stepMapping;
    IPWMDevice *pwmDevice;
    LightModeE mode;
};

class StaircaseFactory {
  public:
    static static_vector<StairStep, MAX_NUM_OF_STEPS>
    createStaircaseFromConfig(const Config &config,
                              static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> &pwmDevices) {
        static_vector<StairStep, MAX_NUM_OF_STEPS> staircases;

        uint8_t globalStepId = 0;
        uint8_t baseI2CAddress = BASE_I2C_ADRESS;

        for (size_t stairIdx = 0; stairIdx < config.stairs.size(); stairIdx++) {
            const auto &stairConfig = config.stairs[stairIdx];
            for (int i = 0; i < stairConfig.stepsCount; i++) {
                uint8_t expanderIndex = globalStepId / STEPS_PER_PWM_DEVICE;
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

FakePWMDevice fakePWM_1(16);
FakePWMDevice fakePWM_2(16);
FakePWMDevice fakePWM_3(16);

static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> fakeDevices = {
    {BASE_I2C_ADRESS, &fakePWM_1},
    {BASE_I2C_ADRESS + 1, &fakePWM_2},
    {BASE_I2C_ADRESS + 2, &fakePWM_3}};

constexpr uint8_t defaultDayYellowBrightness = 80;
constexpr uint8_t defaultDayWhiteBrightness = 60;
constexpr uint8_t defaultNightYellowBrightness = 100;
constexpr uint8_t defaultNightWhiteBrightness = 100;

constexpr uint8_t stairSpecyficDayYellowBrightness = 50;
constexpr uint8_t stairSpecyficDayWhiteBrightness = 40;
constexpr uint8_t stairSpecyficNightYellowBrightness = 100;
constexpr uint8_t stairSpecyficNightWhiteBrightness = 150;

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

    for (size_t i = numberOfStairsWithoutLightMode; i < numberOfStairsWithoutLightMode; i++) {
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
