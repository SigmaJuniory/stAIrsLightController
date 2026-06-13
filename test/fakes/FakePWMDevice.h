#pragma once
#include "interfaces/IPWMDevice.h"
#include <vector>

class FakePWMDevice : public IPWMDevice {
public:
  explicit FakePWMDevice(size_t channelsCount);

  bool begin() override;
  void setPWM(uint8_t channel, uint16_t value) override;
  uint16_t getPWM(uint8_t channel) const;

private:
  std::vector<uint16_t> channels;
};