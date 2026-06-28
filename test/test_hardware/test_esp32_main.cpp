#include <Arduino.h>
#include <unity.h>

void test_led_driver_init();
void test_led_driver_init_error();
void test_led_driver_set_pwm();
void test_led_driver_set_pwm_channel_4();
void test_led_driver_multiple_channels();

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Start testing...");
}

void loop() {
    static bool executed = false;
    if (!executed) {
        UNITY_BEGIN();
        RUN_TEST(test_led_driver_init);
        RUN_TEST(test_led_driver_init_error);
        RUN_TEST(test_led_driver_set_pwm);
        RUN_TEST(test_led_driver_set_pwm_channel_4);
        RUN_TEST(test_led_driver_multiple_channels);
        UNITY_END();
        executed = true;
    }
}
