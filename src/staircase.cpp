#include "staircase.h"

#include <ctime>
#include <stdexcept>

namespace {

std::size_t countConfiguredSteps(const Config &config) {
    std::size_t totalSteps = 0;

    for (const auto &stairConfig : config.stairs) {
        if (stairConfig.stepsCount < 0) {
            throw std::invalid_argument("Configured stair steps count cannot be negative");
        }

        totalSteps += static_cast<std::size_t>(stairConfig.stepsCount);
    }

    return totalSteps;
}

} // namespace

namespace {
PwmValue convertPercentageToPwmValue(BrightnessPercentage percentage) {
    return (MAX_PWM_VALUE * percentage) / 100;
}
} // namespace

StairStep::StairStep(const StepMapping &stepMapping, IPWMDevice *pwmDevice)
    : stepMapping(stepMapping), pwmDevice(pwmDevice), mode(LightModeE::DayMode) {}

void StairStep::updateModeBasedOnTime() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    int hour = timeinfo->tm_hour;
    mode = (hour >= DAY_START && hour < NIGHT_START) ? LightModeE::DayMode : LightModeE::NightMode;
}

void StairStep::setYellow() {
    const auto brightness = mode == LightModeE::DayMode ? stepMapping.dayYellowBrightness
                                                        : stepMapping.nightYellowBrightness;
    uint8_t channelWarm = (stepMapping.stepId % STEPS_PER_PWM_DEVICE) * 2;
    pwmDevice->setPWM(channelWarm, convertPercentageToPwmValue(brightness));
}

void StairStep::setWhite() {
    const auto brightness = mode == LightModeE::DayMode ? stepMapping.dayWhiteBrightness
                                                        : stepMapping.nightWhiteBrightness;
    uint8_t channelCold = (stepMapping.stepId % STEPS_PER_PWM_DEVICE) * 2 + 1;
    pwmDevice->setPWM(channelCold, convertPercentageToPwmValue(brightness));
}

void StairStep::setAll() {
    setYellow();
    setWhite();
}

void StairStep::setMode(LightModeE mode) {
    this->mode = mode;
}

LightModeE StairStep::getMode() const {
    return mode;
}

void StairStep::setWarm() {
    setYellow();
}

StepMapping StairStep::getStepMapping() const {
    return stepMapping;
}

std::size_t StaircaseFactory::countRequiredPwmExpanders(const Config &config) {
    validateConfig(config);

    const std::size_t totalSteps = countConfiguredSteps(config);
    if (totalSteps == 0) {
        return 0;
    }

    return (totalSteps + STEPS_PER_PWM_DEVICE - 1) / STEPS_PER_PWM_DEVICE;
}

void StaircaseFactory::validateConfig(const Config &config) {
    const std::size_t totalSteps = countConfiguredSteps(config);
    if (totalSteps > MAX_NUM_OF_STEPS) {
        throw std::invalid_argument("Configured stair steps exceed the supported maximum");
    }
}

static_vector<StairStep, MAX_NUM_OF_STEPS> StaircaseFactory::createStaircaseFromConfig(
    const Config &config, static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> &pwmDevices) {
    validateConfig(config);

    static_vector<StairStep, MAX_NUM_OF_STEPS> staircases;

    uint8_t globalStepId = 0;

    for (size_t stairIdx = 0; stairIdx < config.stairs.size(); stairIdx++) {
        const auto &stairConfig = config.stairs[stairIdx];
        for (int i = 0; i < stairConfig.stepsCount; i++) {
            uint8_t expanderIndex = globalStepId / STEPS_PER_PWM_DEVICE;
            uint8_t expanderI2CAddress = BASE_I2C_ADRESS + expanderIndex;

            if (!pwmDevices.contains(expanderI2CAddress)) {
                throw std::runtime_error("No PWM device configured for expander address");
            }

            LightMode lightMode = (stairConfig.hasLightMode) ? stairConfig.lightMode
                                                             : config.globalSettings.lightMode;

            StepMapping stepMapping{.stepId = globalStepId,
                                    .stairIndex = static_cast<uint8_t>(stairIdx),
                                    .expanderI2CAddress = expanderI2CAddress,
                                    .dayYellowBrightness = lightMode.day.yellowLightBrightness,
                                    .dayWhiteBrightness = lightMode.day.whiteLightBrightness,
                                    .nightYellowBrightness = lightMode.night.yellowLightBrightness,
                                    .nightWhiteBrightness = lightMode.night.whiteLightBrightness};

            staircases.push_back(StairStep(stepMapping, pwmDevices.at(expanderI2CAddress)));
            globalStepId++;
        }
    }

    return staircases;
}
