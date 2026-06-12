#include "fakes/FakePWMDevice.h"

FakePWMDevice::FakePWMDevice(size_t channelsCount) : channels(channelsCount, 0) {
}

bool FakePWMDevice::begin() {
  return true;
}

void FakePWMDevice::setPWM(uint8_t channel, uint16_t value) {
  if (channel < channels.size()) {
    channels[channel] = value;
  }
}