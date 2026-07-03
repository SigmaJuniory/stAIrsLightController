#pragma once

#include "pwm_device.h"
#include "staircase.h"
#include "static_map.h"

#include <array>
#include <cstddef>

constexpr std::size_t MAX_PWM_EXPANDERS =
    (MAX_NUM_OF_STEPS + STEPS_PER_PWM_DEVICE - 1) / STEPS_PER_PWM_DEVICE;

using PwmExpanderMap = static_map<uint8_t, IPWMDevice *, MAX_NUM_OF_STEPS>;

class PwmExpanderRegistry {
  public:
    void createPwmExpanders(TwoWire &wire);
    void initializePwmExpanders(std::size_t requiredExpandersCount);
    PwmExpanderMap &getPwmExpanderMap();

  private:
    using PwmExpanderArray = std::array<PWMDevice, MAX_PWM_EXPANDERS>;

    PwmExpanderArray pwmExpanders_{};
    PwmExpanderMap pwmExpandersByAddress_{};
};
