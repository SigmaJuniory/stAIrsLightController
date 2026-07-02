#pragma once

#include "config_parser.h"
#include "interfaces/ipwm_device.h"
#include "static_map.h"
#include "static_vector.h"

#include <cstddef>
#include <cstdint>

constexpr uint8_t CHANNELS_PER_EXPANDER = 16;
constexpr uint8_t BASE_I2C_ADRESS = 0x40;
constexpr uint8_t CHANNELS_PER_STEP = 2;
constexpr uint8_t STEPS_PER_PWM_DEVICE = CHANNELS_PER_EXPANDER / CHANNELS_PER_STEP;
constexpr uint8_t DAY_START = 7;
constexpr uint8_t NIGHT_START = 19;
constexpr uint8_t MAX_NUM_OF_STEPS = 64;

enum class LightModeE : uint8_t { DayMode, NightMode };

struct StepMapping {
    uint8_t stepId;
    uint8_t stairIndex;
    uint8_t expanderI2CAddress;
    BrightnessPercentage dayYellowBrightness;
    BrightnessPercentage dayWhiteBrightness;
    BrightnessPercentage nightYellowBrightness;
    BrightnessPercentage nightWhiteBrightness;
};

class StairStep {
  public:
    StairStep() = default;
    explicit StairStep(const StepMapping &stepMapping, IPWMDevice *pwmDevice);

    void updateModeBasedOnTime();
    void setYellow();
    void setWhite();
    void setAll();
    void setMode(LightModeE mode);
    LightModeE getMode() const;
    void setWarm();
    StepMapping getStepMapping() const;

  private:
    StepMapping stepMapping{};
    IPWMDevice *pwmDevice{};
    LightModeE mode{LightModeE::DayMode};
};

class StaircaseFactory {
  public:
    static static_vector<StairStep, MAX_NUM_OF_STEPS>
    createStaircaseFromConfig(const Config &config,
                              static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> &pwmDevices);
};
