#include <Arduino.h>
#include <Wire.h>

#include "config_parser.h"
#include "pwm_device.h"
#include "staircase.h"

PWMDevice pwm(0x40, Wire);

static_vector<StairStep, MAX_NUM_OF_STEPS> stairSteps;
bool staircaseReady = false;

// TODO: Add more steps to the config, and align Wokwi with it.
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

    Wire.begin(21, 20);

    if (!pwm.begin()) {
        Serial.println("PCA9685 init failed");
        return;
    }

    pwm.setPWMFreq(1000);

    Config config = ConfigParser::parseConfigFromJson(demoConfig);
    // TODO: Implement dynamic creation of the pwmDevices map from the parsed config.
    static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS> pwmDevices = {{BASE_I2C_ADRESS, &pwm}};
    stairSteps = StaircaseFactory::createStaircaseFromConfig(config, pwmDevices);

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
