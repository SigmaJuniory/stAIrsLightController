#include "LedDriver.h"
#include <Arduino.h>
#include <gtest/gtest.h>

class LedDriverTestBase : public ::testing::Test {
  protected:
    LedDriver *ledDriver = nullptr;

    void SetUp() override {
        Wire.setPins(13, 14);
        Wire.begin();
    }

    void TearDown() override {
        if (ledDriver != nullptr) {
            for (int i = 0; i < 16; i++) {
                ledDriver->setPWM(i, 0, 0);
            }
            delete ledDriver;
            ledDriver = nullptr;
        }
    }
};

// Klasa z automatyczną inicjalizacją - dla większości testów
class LedDriverTestWithInit : public LedDriverTestBase {
  protected:
    LedDriver *ledDriver = nullptr;

    void SetUp() override {
        // Najpierw inicjalizujemy I2C
        LedDriverTestBase::SetUp();

        // Potem tworzymy i inicjalizujemy driver
        ledDriver = new LedDriver(0x40, Wire);
        bool init = ledDriver->begin();
        ASSERT_TRUE(init) << "Inicjalizacja nie powiodła się!";

        ledDriver->setPWMFreq(1000);

        for (int i = 0; i < 16; i++) {
            ledDriver->setPWM(i, 0, 0);
        }
        delay(10);
    }
};

TEST_F(LedDriverTestBase, TestInit) {
    ledDriver = new LedDriver(0x40, Wire);
    bool init = ledDriver->begin();
    EXPECT_TRUE(init);
}

TEST_F(LedDriverTestBase, TestInitError) {
    ledDriver = new LedDriver(0x50, Wire);
    bool init = ledDriver->begin();
    EXPECT_FALSE(init);
}

TEST_F(LedDriverTestWithInit, SetPWM) {
    ledDriver->setPWM(0, 0, 2048);
    delay(10);
    EXPECT_EQ(2048, ledDriver->getPWM(0, true));
}

TEST_F(LedDriverTestWithInit, MultipleChannels) {
    for (int i = 0; i < 16; i++) {
        ledDriver->setPWM(i, 0, i * 256);
    }
    delay(10);

    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(i * 256, ledDriver->getPWM(i, true));
    }
}
