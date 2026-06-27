#pragma once

#include "interfaces/ipwm_device.h"

#include <Adafruit_PWMServoDriver.h>

class PWMDevice : public IPWMDevice {
  public:
    PWMDevice(const uint8_t addr, TwoWire &i2c);

    bool begin() override;
    void setPWMFreq(float freq);
    uint16_t getPWM(uint8_t channel);
    void setPWM(uint8_t channel, uint16_t val) override;

  private:
    Adafruit_PWMServoDriver pwmDriver_;
};
