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

#include <ESP8266WiFi.h>
#include "GY25.h"
#include "CommandServer.h"
#include "MotorHandler.h"

GY25 gy25;
MotorHandler motors(5, 4, 450); // D1, D2
CommandServer server(&motors, &gy25);
unsigned long lastRotaryMillis = 0;
int lastVal = 0;
unsigned long systemStartMillis = 0;
bool onceOn = false;
bool onceOff = false;

bool setupWifi()
{
    WiFi.mode(WIFI_AP);
    
    bool b1 = WiFi.softAP("Luftfaltung", "QWW_488_Veritas_Twoggle");
    delay(100);
    bool b2 = WiFi.softAPConfig(IPAddress(192,168,121,1), IPAddress(192,168,121,254), IPAddress(255,255,255,0));

    if (b1 && b2) {
      Serial.print("AP started ");
      Serial.println(WiFi.softAPIP());
      
      return true;
    } else {
      Serial.println("Could not start AP. start: "+String(b1)+" config:"+String(b2));

      return false;
    }
}

void setup() {
  Serial.begin(115200);
  
  //gy25.init();

/*
  if (setupWifi()) {
    server.begin();
  }*/

  motors.begin();

  systemStartMillis = millis();
  Serial.println("Looping");
}

void loop() {
  unsigned long now = millis();

  if (now - systemStartMillis > 15*1000 && !onceOn) {
    motors.thrust(170);
    onceOn = true;
  }

  if (now - systemStartMillis > 45*1000 && !onceOff) {
    motors.thrust(0);
    onceOff = true;
  }

  motors.drive();

  //server.drive();
  bool hasNewOri = false; //gy25.drive();

  bool doSomething = now - lastRotaryMillis >= 3000;

/*
  if (doSomething) {
    Serial.print(""+String(WiFi.softAPgetStationNum()));
    lastRotaryMillis = now;
  }*/

  
  if (doSomething && hasNewOri) {
    float ori = gy25.getRoll();

    // NOTE too frequent analog reading breaks Wifi (???!)
    float range = analogRead(A0) / 1023.0; // 0..1
    if (range <= 0.02f) {
      range = 0;
    }

    int pwmVal = 1000 + (int)(range* 350);

    Serial.println("O "+String(ori)+" P "+String(gy25.getPitch()));

    /*
    if (lastVal != pwmVal) {
      motors.thrust(pwmVal);
      lastVal = pwmVal;
    }*/

    lastRotaryMillis = now;
  }

  //digitalWrite(LED, now % 1000 > 499 ? HIGH : LOW);
  //delay(1000); // must read mcu continously
  delayMicroseconds(500);
}
