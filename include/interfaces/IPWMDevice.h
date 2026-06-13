#pragma once
#include <cstdint>

class IPWMDevice {
public:
  virtual ~IPWMDevice() = default;

  virtual bool begin() = 0;
  virtual void setPWM(uint8_t channel, uint16_t value) = 0;
};