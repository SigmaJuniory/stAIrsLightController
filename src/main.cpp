#include <Arduino.h>
#include <Wire.h>

#include <array>

#include "config_parser.h"
#include "pwm_device.h"
#include "staircase.h"

static_vector<StairStep, MAX_NUM_OF_STEPS> stairSteps;
bool staircaseReady = false;

// TODO: Add more steps to the demo config.
constexpr auto demoConfig = R"({
  "globalSettings": {
    "lightMode": {
      "day": {
        "YellowLightBrightness": 2000,
        "WhiteLightBrightness": 1800
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

void setup() {
    Serial.begin(115200);

    Wire.begin(13, 14);

    try {
        Config config = ConfigParser::parseConfigFromJson(demoConfig);

        constexpr std::size_t maxPwmExpanders = MAX_NUM_OF_STEPS / STEPS_PER_PWM_DEVICE;
        std::array<PWMDevice, maxPwmExpanders> pwmExpanders = {
            PWMDevice(BASE_I2C_ADRESS + 0, Wire), PWMDevice(BASE_I2C_ADRESS + 1, Wire),
            PWMDevice(BASE_I2C_ADRESS + 2, Wire), PWMDevice(BASE_I2C_ADRESS + 3, Wire),
            PWMDevice(BASE_I2C_ADRESS + 4, Wire), PWMDevice(BASE_I2C_ADRESS + 5, Wire),
            PWMDevice(BASE_I2C_ADRESS + 6, Wire), PWMDevice(BASE_I2C_ADRESS + 7, Wire)};

        static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> pwmDevices;
        const auto requiredExpanders = StaircaseFactory::countRequiredPwmExpanders(config);

        for (std::size_t expanderIndex = 0; expanderIndex < requiredExpanders; ++expanderIndex) {
            auto &expander = pwmExpanders[expanderIndex];
            if (!expander.begin()) {
                Serial.printf("PCA9685 init failed for expander 0x%02x\n",
                              static_cast<unsigned>(BASE_I2C_ADRESS + expanderIndex));
                return;
            }

            expander.setPWMFreq(1000);
            pwmDevices.insert(BASE_I2C_ADRESS + expanderIndex, &expander);
        }

        stairSteps = StaircaseFactory::createStaircaseFromConfig(config, pwmDevices);

        staircaseReady = stairSteps.size() > 0;
        Serial.printf("Loaded %u demo steps from config\n",
                      static_cast<unsigned>(stairSteps.size()));

        for (size_t i = 0; i < stairSteps.size(); i++) {
            const StepMapping mapping = stairSteps[i].getStepMapping();
            const uint8_t yellowChannel =
                (mapping.stepId % STEPS_PER_PWM_DEVICE) * CHANNELS_PER_STEP;
            const uint8_t whiteChannel = yellowChannel + 1;

            Serial.printf("Step %u -> PCA9685 0x%02x, yellow channel %u, white channel %u\n",
                          static_cast<unsigned>(mapping.stepId), mapping.expanderI2CAddress,
                          yellowChannel, whiteChannel);
        }

        size_t activeStep = stairSteps.size() - 1;
        stairSteps[activeStep].setMode(LightModeE::DayMode);
        stairSteps[activeStep].setAll();
    } catch (const std::exception &exception) {
        Serial.printf("Configuration error: %s\n", exception.what());
    }

    staircaseReady = stairSteps.size() > 0;
    Serial.printf("Loaded %u demo steps from config\n", static_cast<unsigned>(stairSteps.size()));

    for (size_t i = 0; i < stairSteps.size(); i++) {
        const StepMapping mapping = stairSteps[i].getStepMapping();
        const uint8_t yellowChannel = (mapping.stepId % STEPS_PER_PWM_DEVICE) * CHANNELS_PER_STEP;
        const uint8_t whiteChannel = yellowChannel + 1;

        Serial.printf("Step %u -> PCA9685 0x%02x, yellow channel %u, white channel %u\n",
                      static_cast<unsigned>(mapping.stepId), mapping.expanderI2CAddress,
                      yellowChannel, whiteChannel);
    }

    size_t activeStep = stairSteps.size() - 1;
    stairSteps[activeStep].setMode(LightModeE::DayMode);
    stairSteps[activeStep].setAll();
}

void loop() {}
