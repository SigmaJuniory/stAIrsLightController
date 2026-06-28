#pragma once

#include "pwm_types.h"

#include <cstdint>

class IPWMDevice {
  public:
    virtual ~IPWMDevice() = default;

    virtual bool begin() = 0;
    virtual void setPWM(uint8_t channel, PwmValue value) = 0;
};
