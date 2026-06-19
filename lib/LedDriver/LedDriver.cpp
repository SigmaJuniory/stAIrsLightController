#include "LedDriver.h"

// LedDriver::LedDriver() : Adafruit_PWMServoDriver() {}

// LedDriver::LedDriver(const uint8_t addr) : Adafruit_PWMServoDriver(addr) {}

/*!
 *  @brief  Instantiates a new Led driver chip with the I2C address on a
 * TwoWire interface
 *  @param  addr The 7-bit I2C address to locate this chip, default is 0x40
 *  @param  i2c  A reference to a 'TwoWire' object that we'll use to communicate
 *  with
 */
LedDriver::LedDriver(const uint8_t addr, TwoWire &i2c) : Adafruit_PWMServoDriver(addr, i2c) {}
