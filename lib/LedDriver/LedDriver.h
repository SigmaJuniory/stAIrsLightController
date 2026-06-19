#ifndef LED_DRIVER_HH
#define LED_DRIVER_HH

#include <Adafruit_PWMServoDriver.h>

class LedDriver : public Adafruit_PWMServoDriver {
  public:
    // LedDriver();
    // LedDriver(const uint8_t addr);
    LedDriver(const uint8_t addr, TwoWire &i2c);
};

#endif
