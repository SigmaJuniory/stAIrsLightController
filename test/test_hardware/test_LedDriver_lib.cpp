#include "pwm_device.h"
#include <Arduino.h>
#include <gtest/gtest.h>
// TODO: Use Unity instead gtest for hardware tests

namespace {
constexpr uint8_t i2cSdaPin = 21; // TODO: Make this correct for ESP32 and Wokwi.
constexpr uint8_t i2cSclPin = 20;
bool wireInitialized = false;
} // namespace

class LedDriverTestBase : public ::testing::Test {
  protected:
    PWMDevice *ledDriver = nullptr;

    void SetUp() override {
        initWire();
    }

    void TearDown() override {
        if (ledDriver != nullptr) {
            for (int i = 0; i < 16; i++) {
                ledDriver->setPWM(i, 0);
            }
            delete ledDriver;
            ledDriver = nullptr;
        }
    }

    void initWire() {
        if (!wireInitialized) {
            Wire.begin(i2cSdaPin, i2cSclPin);
            wireInitialized = true;
        }
    }

    void expectPwmValue(uint8_t channel, uint16_t expected) {
#ifdef WOKWI_SIMULATOR_TEST
        // TODO: Remove this branch after adding PWM register readback to the
        // Wokwi PCA9685 custom chip fork.
        (void)channel;
        (void)expected;
        SUCCEED();
#else
        EXPECT_EQ(expected, ledDriver->getPWM(channel));
#endif
    }
};

class LedDriverTestWithInit : public LedDriverTestBase {
  protected:
    void SetUp() override {
        initWire();
        // TODO: use unique_ptr or static inicialisation
        ledDriver = new PWMDevice(0x40, Wire);
        bool init = ledDriver->begin();
        ASSERT_TRUE(init) << "Inicialisation of PWMDevice failed!";

        ledDriver->setPWMFreq(1000);

        for (int i = 0; i < 16; i++) {
            ledDriver->setPWM(i, 0);
        }
        delay(10);
    }
};

TEST_F(LedDriverTestBase, TestInit) {
    ledDriver = new PWMDevice(0x40, Wire);
    bool init = ledDriver->begin();
    EXPECT_TRUE(init);
}

TEST_F(LedDriverTestBase, TestInitError) {
    ledDriver = new PWMDevice(0x50, Wire);
    bool init = ledDriver->begin();
    EXPECT_FALSE(init);
}

TEST_F(LedDriverTestWithInit, SetPWM) {
    ledDriver->setPWM(0, 2048);
    delay(10);
    expectPwmValue(0, 2048);
}

TEST_F(LedDriverTestWithInit, SetPWM_2) {
    ledDriver->setPWM(4, 1000);
    delay(10);
    expectPwmValue(4, 1000);
}

TEST_F(LedDriverTestWithInit, MultipleChannels) {
    for (int i = 0; i < 16; i++) {
        delay(50);
        ledDriver->setPWM(i, i + 1);
    }
    delay(1000);

    for (int i = 0; i < 16; i++) {
        expectPwmValue(i, i + 1);
    }
}
