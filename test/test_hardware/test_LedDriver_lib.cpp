#include "pwm_device.h"
#include <Arduino.h>
#include <gtest/gtest.h>
// TODO: Use Unity instead gtest for hardware tests

class LedDriverTestBase : public ::testing::Test {
  protected:
    PWMDevice *ledDriver = nullptr;
    bool wireInitialized = false;

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
            Wire.setPins(13, 14);
            Wire.begin();
            wireInitialized = true;
        }
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
    EXPECT_EQ(2048, ledDriver->getPWM(0));
}

TEST_F(LedDriverTestWithInit, SetPWM_2) {
    ledDriver->setPWM(4, 1000);
    delay(10);
    EXPECT_EQ(1000, ledDriver->getPWM(4));
}

TEST_F(LedDriverTestWithInit, MultipleChannels) {
    for (int i = 0; i < 16; i++) {
        delay(50);
        ledDriver->setPWM(i, i + 1);
    }
    delay(1000);

    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(i + 1, ledDriver->getPWM(i));
    }
}
