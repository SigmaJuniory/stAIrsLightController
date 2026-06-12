#pragma once
#include "interfaces/IPWMDevice.h"
#include <array>

class FakePWMDevice : public IPWMDevice {
public:
  bool begin() override {
    return true; // Simulate successful initialization
  }

  void setPWM(uint8_t channel, uint16_t value) override {
    // Simulate setting PWM value (no actual hardware interaction)
    channels[channel] = value;
  }

private:
  std::array<uint16_t, 8> channels;
};