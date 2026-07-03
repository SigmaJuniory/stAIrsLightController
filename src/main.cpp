#include <Arduino.h>
#include <Wire.h>

#include <stdexcept>

#include "config_parser.h"
#include "pwm_expander_registry.h"
#include "staircase.h"

namespace {

static_vector<StairStep, MAX_NUM_OF_STEPS> stairSteps;
PwmExpanderRegistry pwmExpanderRegistry;

void printStepMappings() {
    Serial.printf("Loaded %u demo steps from config\n", static_cast<unsigned>(stairSteps.size()));

    for (size_t i = 0; i < stairSteps.size(); i++) {
        const StepMapping mapping = stairSteps[i].getStepMapping();
        const uint8_t yellowChannel = (mapping.stepId % STEPS_PER_PWM_DEVICE) * CHANNELS_PER_STEP;
        const uint8_t whiteChannel = yellowChannel + 1;

        Serial.printf("Step %u -> PCA9685 0x%02x, yellow channel %u, white channel %u\n",
                      static_cast<unsigned>(mapping.stepId), mapping.expanderI2CAddress,
                      yellowChannel, whiteChannel);
    }
}

void activateLastDemoStep() {
    if (stairSteps.size() == 0) {
        return;
    }

    const size_t activeStep = stairSteps.size() - 1;
    stairSteps[activeStep].setMode(LightModeE::DayMode);
    stairSteps[activeStep].setAll();
}

// TODO: Add more steps to the demo config.
constexpr auto demoConfig = R"({
  "globalSettings": {
    "lightMode": {
      "day": {
        "YellowLightBrightness": 80,
        "WhiteLightBrightness": 60
      },
      "night": {
        "YellowLightBrightness": 80,
        "WhiteLightBrightness": 30
      }
    }
  },
  "stairs": [
    {
      "stepsCount": 1
    }
  ]
})";

void updateStairSteps() {
    Config config = ConfigParser::parseConfigFromJson(demoConfig);

    pwmExpanderRegistry.initializePwmExpanders(StaircaseFactory::countRequiredPwmExpanders(config));
    stairSteps = StaircaseFactory::createStaircaseFromConfig(
        config, pwmExpanderRegistry.getPwmExpanderMap());
}

} // namespace

void setup() {
    Serial.begin(115200);
    Wire.begin(13, 14);
    pwmExpanderRegistry.createPwmExpanders(Wire);

    try {
        updateStairSteps();
        printStepMappings();
        activateLastDemoStep();
    } catch (const std::exception &exception) {
        Serial.printf("Configuration error: %s\n", exception.what());
    }
}

void loop() {}
