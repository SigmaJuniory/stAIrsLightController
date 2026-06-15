#include <Arduino.h>
#include <gtest/gtest.h>
// uncomment line below if you plan to use GMock
// #include <gmock/gmock.h>

// TEST(...)
// TEST_F(...)

TEST(Sanity, TrueIsTrue) {
  EXPECT_TRUE(true);
}

TEST(Sanity, IsItFalse) {
  EXPECT_FALSE(false);
}

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
