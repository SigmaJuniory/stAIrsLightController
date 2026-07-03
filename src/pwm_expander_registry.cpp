#include "pwm_expander_registry.h"

#include <Arduino.h>

#include <stdexcept>

void PwmExpanderRegistry::createPwmExpanders(TwoWire &wire) {
    for (std::size_t i = 0; i < MAX_PWM_EXPANDERS; ++i) {
        pwmExpanders_[i] = PWMDevice(BASE_I2C_ADRESS + i, wire);
    }
}

void PwmExpanderRegistry::initializePwmExpanders(std::size_t requiredExpandersCount) {
    for (std::size_t expanderIndex = 0; expanderIndex < requiredExpandersCount; ++expanderIndex) {
        auto &pwmExpander = pwmExpanders_[expanderIndex];
        if (!pwmExpander.begin()) {
            Serial.printf("PCA9685 init failed for expander 0x%02x\n",
                          static_cast<unsigned>(BASE_I2C_ADRESS + expanderIndex));
            throw std::runtime_error("PCA9685 init failed for expander");
        }

        pwmExpander.setPWMFreq(1000);
        pwmExpandersByAddress_.insert(static_cast<uint8_t>(BASE_I2C_ADRESS + expanderIndex),
                                      &pwmExpander);
    }
}

PwmExpanderMap &PwmExpanderRegistry::getPwmExpanderMap() {
    return pwmExpandersByAddress_;
}
