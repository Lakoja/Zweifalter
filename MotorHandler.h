/*
 * Copyright (C) 2018 Lakoja on github.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MOTOR_HANDLER_H__
#define __MOTOR_HANDLER_H__

#include <Servo.h>

class MotorHandler
{
private:
  const int BASE_POWER = 1000;

  int motorPin1;
  int motorPin2;
  int maxPower;
  Servo motor1;
  Servo motor2;
  unsigned int savedPower1 = 0;
  unsigned int savedPower2 = 0;
  unsigned int lastSetPower1 = 0;
  unsigned int lastSetPower2 = 0;
  bool isActive = true;
  unsigned long lastStartMillis = 0;
  unsigned long lastForcedOffMillis = 0;

public:
  MotorHandler(int pin1, int pin2, int maxi)
  {
    motorPin1 = pin1;
    motorPin2 = pin2;
    maxPower = maxi;
  }

  void begin()
  {
    motor1.attach(motorPin1);
    motor1.write(1000); // otherwise 1500 is default
    motor2.attach(motorPin2);
    motor2.write(1000);
  }

  void drive()
  {
    unsigned long now = millis();
    if (lastStartMillis > 0) {
      if (now - lastStartMillis >= 45*1000) {
        setActive(false);
        lastForcedOffMillis = now;
      }
    } else {
      if (!isActive && now - lastForcedOffMillis >= 15*1000) {
        setActive(true);
        lastForcedOffMillis = 0;
      }
    }
  }

  // power: 0..1000
  void thrust(unsigned int power)
  {
    thrust1(power);
    thrust2(power);
  }

  void thrust1(unsigned int power)
  {
    if (power > 1000) {
      power = 1000;
    }
    
    if (!isActive) {
      savedPower1 = power;
      return;
    }
    
    if (power == lastSetPower1) {
      return;
    }

    if (power > 0) {
      if (lastStartMillis == 0) {
        lastStartMillis = millis();
      }
    } else {
      if (lastSetPower2 == 0) {
        lastStartMillis = 0;
      }
    }

    lastSetPower1 = power;
    
    int val = (int)(power / 1000.0f * maxPower);
    
    Serial.print("t"+String(val));
    
    motor1.write(BASE_POWER + val);
  }
  
  void thrust2(unsigned int power)
  {
    if (power > 1000) {
      power = 1000;
    }
    
    if (!isActive) {
      savedPower2 = power;
      return;
    }

    if (power == lastSetPower2) {
      return;
    }

    if (power > 0) {
      if (lastStartMillis == 0) {
        lastStartMillis = millis();
      }
    } else {
      if (lastSetPower1 == 0) {
        lastStartMillis = 0;
      }
    }

    lastSetPower2 = power;
    
    int val = (int)(power / 1000.0f * maxPower);
    
    motor2.write(BASE_POWER + val);
  }

private:
  void setActive(bool active)
  {
    if (active == isActive) {
      return;
    }
    
    if (active) {
      Serial.println("On");
      isActive = true;
      
      thrust1(savedPower1);
      thrust2(savedPower2);
    } else {
      savedPower1 = lastSetPower1;
      savedPower2 = lastSetPower2;

      Serial.println("Off");
      
      thrust1(0);
      thrust2(0);

      isActive = false;
    }
  }
};

#endif
