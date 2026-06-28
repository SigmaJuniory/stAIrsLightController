#include "fakes/fake_pwm_device.h"
#include <algorithm>

FakePWMDevice::FakePWMDevice(size_t channelsCount) : channels(channelsCount, 0) {}

bool FakePWMDevice::begin() {
    return true;
}

void FakePWMDevice::setPWM(uint8_t channel, PwmValue value) {
    if (channel < channels.size()) {
        channels[channel] = std::min(value, MAX_PWM);
    }
}

PwmValue FakePWMDevice::getPWM(uint8_t channel) const {
    if (channel < channels.size()) {
        return channels[channel];
    }
    return 0;
}
