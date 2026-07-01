#pragma once

#include "interfaces/ipwm_device.h"

#include <Adafruit_PWMServoDriver.h>

class PWMDevice : public IPWMDevice {
  public:
    PWMDevice(const uint8_t addr, TwoWire &i2c);

    bool begin() override;
    void setPWMFreq(float freq);
    PwmValue getPWM(uint8_t channel);
    void setPWM(uint8_t channel, PwmValue val) override;

  private:
    Adafruit_PWMServoDriver pwmDriver_;
};
