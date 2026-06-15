#include "fakes/FakePWMDevice.h"
#include <algorithm>

FakePWMDevice::FakePWMDevice(size_t channelsCount) : channels(channelsCount, 0) {}

bool FakePWMDevice::begin() {
  return true;
}

void FakePWMDevice::setPWM(uint8_t channel, uint16_t value) {
  if (channel < channels.size()) {
    channels[channel] = std::min(value, MAX_PWM);
  }
}

uint16_t FakePWMDevice::getPWM(uint8_t channel) const {
  if (channel < channels.size()) {
    return channels[channel];
  }
  return 0;
}
