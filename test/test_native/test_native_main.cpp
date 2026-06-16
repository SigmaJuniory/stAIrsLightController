#include "fakes/FakePWMDevice.h"
#include <gtest/gtest.h>

#if defined(ARDUINO)
#include <Arduino.h>

void setup() {
    Serial.begin(115200);

    ::testing::InitGoogleTest();
}

void loop() {
    static bool executed = false;
    if (!executed) {
        (void)RUN_ALL_TESTS();
        executed = true;
    }
}

#else
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
