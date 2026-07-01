#pragma once
#include "interfaces/ipwm_device.h"
#include <vector>

class FakePWMDevice : public IPWMDevice {
  public:
    explicit FakePWMDevice(size_t channelsCount);

    bool begin() override;
    void setPWM(uint8_t channel, PwmValue value) override;
    PwmValue getPWM(uint8_t channel) const;

    static constexpr PwmValue MAX_PWM = 4095;

  private:
    std::vector<PwmValue> channels;
};
