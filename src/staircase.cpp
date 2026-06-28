#include "staircase.h"

#include <ctime>

StairStep::StairStep(const StepMapping &stepMapping, IPWMDevice *pwmDevice)
    : stepMapping(stepMapping), pwmDevice(pwmDevice), mode(LightModeE::DayMode) {}

void StairStep::updateModeBasedOnTime() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    int hour = timeinfo->tm_hour;
    mode = (hour >= DAY_START && hour < NIGHT_START) ? LightModeE::DayMode : LightModeE::NightMode;
}

void StairStep::setYellow() {
    PwmValue brightness = mode == LightModeE::DayMode ? stepMapping.dayYellowBrightness
                                                      : stepMapping.nightYellowBrightness;
    uint8_t channelWarm = (stepMapping.stepId % STEPS_PER_PWM_DEVICE) * 2;
    pwmDevice->setPWM(channelWarm, brightness);
}

void StairStep::setWhite() {
    PwmValue brightness = mode == LightModeE::DayMode ? stepMapping.dayWhiteBrightness
                                                      : stepMapping.nightWhiteBrightness;
    uint8_t channelCold = (stepMapping.stepId % STEPS_PER_PWM_DEVICE) * 2 + 1;
    pwmDevice->setPWM(channelCold, brightness);
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

static_vector<StairStep, MAX_NUM_OF_STEPS> StaircaseFactory::createStaircaseFromConfig(
    const Config &config, static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> &pwmDevices) {
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

            StepMapping stepMapping{.stepId = globalStepId,
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
