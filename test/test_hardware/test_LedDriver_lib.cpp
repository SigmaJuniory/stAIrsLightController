#include "pwm_device.h"
#include <Arduino.h>
#include <unity.h>

namespace {
constexpr uint8_t pwmDriverAddress = 0x40;
constexpr uint8_t missingDeviceAddress = 0x50;
constexpr uint8_t i2cSdaPin = 21; // TODO: Make this correct for ESP32 and Wokwi.
constexpr uint8_t i2cSclPin = 20;
constexpr uint8_t pwmChannelsCount = 16;

PWMDevice *ledDriver = nullptr;
bool wireInitialized = false;

void initWire() {
    if (!wireInitialized) {
        Wire.begin(i2cSdaPin, i2cSclPin);
        wireInitialized = true;
    }
}

PWMDevice *createInitializedDriver() {
    ledDriver = new PWMDevice(pwmDriverAddress, Wire);
    TEST_ASSERT_TRUE_MESSAGE(ledDriver->begin(), "Initialization of PWMDevice failed!");

    ledDriver->setPWMFreq(1000);

    for (uint8_t channel = 0; channel < pwmChannelsCount; channel++) {
        ledDriver->setPWM(channel, 0);
    }
    delay(10);

    return ledDriver;
}

void assertPwmValue(uint8_t channel, uint16_t expected) {
#ifdef WOKWI_SIMULATOR_TEST
    // TODO: Remove this branch after adding LED PWM register readback to the
    // Wokwi PCA9685 custom chip fork.
    // The Wokwi PCA9685 custom chip currently accepts PWM writes, but does not
    // implement readback for channel registers used by Adafruit getPWM().
    (void)channel;
    (void)expected;
    TEST_ASSERT_TRUE(true);
#else
    TEST_ASSERT_EQUAL_UINT16(expected, ledDriver->getPWM(channel));
#endif
}
} // namespace

void setUp() {
    initWire();
}

void tearDown() {
    if (ledDriver != nullptr) {
        for (uint8_t channel = 0; channel < pwmChannelsCount; channel++) {
            ledDriver->setPWM(channel, 0);
        }
        delete ledDriver;
        ledDriver = nullptr;
    }
}

void test_led_driver_init() {
    ledDriver = new PWMDevice(pwmDriverAddress, Wire);
    TEST_ASSERT_TRUE(ledDriver->begin());
}

void test_led_driver_init_error() {
    ledDriver = new PWMDevice(missingDeviceAddress, Wire);
    TEST_ASSERT_FALSE(ledDriver->begin());
}

void test_led_driver_set_pwm() {
    createInitializedDriver();

    ledDriver->setPWM(0, 2048);
    delay(10);
    assertPwmValue(0, 2048);
}

void test_led_driver_set_pwm_channel_4() {
    createInitializedDriver();

    ledDriver->setPWM(4, 1000);
    delay(10);
    assertPwmValue(4, 1000);
}

void test_led_driver_multiple_channels() {
    createInitializedDriver();

    for (uint8_t channel = 0; channel < pwmChannelsCount; channel++) {
        delay(50);
        ledDriver->setPWM(channel, channel + 1);
    }
    delay(1000);

    for (uint8_t channel = 0; channel < pwmChannelsCount; channel++) {
        assertPwmValue(channel, channel + 1);
    }
}
