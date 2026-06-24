#include <Arduino.h>
#include <Wire.h>
#include "pwm_device.h"

PWMDevice pwm(0x40, Wire);

void setup() {
  Serial.begin(115200);

  Wire.begin(21,20);

  if (!pwm.begin()) {
    Serial.println("FAIL");
    return;
  }

  pwm.setPWMFreq(1000);

  // stopień 0
  pwm.setPWM(0, 2000);

  pwm.setPWM(1, 2000);
}

void loop() {}