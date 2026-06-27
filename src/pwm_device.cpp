#include "pwm_device.h"

PWMDevice::PWMDevice(const uint8_t addr, TwoWire &i2c) : pwmDriver_(addr, i2c) {}

bool PWMDevice::begin() {
    return pwmDriver_.begin();
}

void PWMDevice::setPWMFreq(float freq) {
    pwmDriver_.setPWMFreq(freq);
}

uint16_t PWMDevice::getPWM(uint8_t channel) {
    return pwmDriver_.getPWM(channel, true);
}

void PWMDevice::setPWM(uint8_t channel, uint16_t val) {
    pwmDriver_.setPin(channel, val, false);
}
