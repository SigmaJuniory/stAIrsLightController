#pragma once
#include "interfaces/IPWMDevice.h"
#include <vector>

class FakePWMDevice : public IPWMDevice {
  public:
    explicit FakePWMDevice(size_t channelsCount);

    bool begin() override;
    void setPWM(uint8_t channel, uint16_t value) override;
    uint16_t getPWM(uint8_t channel) const;

    static constexpr uint16_t MAX_PWM = 4095;

  private:
    std::vector<uint16_t> channels;
};
