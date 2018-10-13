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

#ifndef __COMMAND_SERVER_H__
#define __COMMAND_SERVER_H__

#include <WiFiServer.h>
#include <WiFiClient.h>
#include "MotorHandler.h"
#include "GY25.h"

class CommandServer : public WiFiServer
{
private:
  const int LED_PIN = 16;
  const int OFF = HIGH;
  const int ON = LOW;
  String receivedLine = "";
  bool clientConnected = false;
  WiFiClient client;
  MotorHandler* motors;
  GY25* gy25;
  unsigned long lastLedOn = -1;
  
public:
  CommandServer(MotorHandler* mh, GY25* gy) : WiFiServer(80)
  {
    motors = mh;
    gy25 = gy;
  }

  void begin()
  {
    WiFiServer::begin();

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, OFF);
  }

  void drive()
  {
    unsigned long driveStart = millis();

    handleLed();

    if (!client.connected()) {
      client = available();
    }
  
    if (client.connected()) {
      if (!clientConnected) {
        Serial.println("Client");
        //client.setNoDelay(true);
        clientConnected = true;
        receivedLine = "";
      }

      while (client.available() > 0 && millis() - driveStart > 0) {
        char c = client.read();

        if (c == '\n') {
          if (receivedLine.length() > 0) {
            //Serial.println("L "+receivedLine);

            handleCommand(receivedLine);
            
            receivedLine = "";
          }
        } else {
          receivedLine += c;
        }
      }
    } else if (clientConnected) {
      Serial.println("Client lost");
      clientConnected = false;
    }
  }

private:
  void handleCommand(String command)
  {
    handleLed(true);
    
    if (command.startsWith("STATUS")) {
       client.println("ORI "+String((int)(gy25->getRoll() * 10 + 0.5f))+" "+String((int)(gy25->getPitch() * 10 + 0.5f)));
       client.flush();
    } else if (command.startsWith("MOVE ")) {
      
    } else if (command.startsWith("THRUST ")) {
      String numberPart = command.substring(7);
      int power = numberPart.toInt();
      //Serial.println("T "+String(power));
      motors->thrust(power);
    } else {
      Serial.println("!! Command not understood: "+command);
    }
  }

  void handleLed(bool activate = false)
  {
    unsigned long now = millis();
    if (activate) {
      digitalWrite(LED_PIN, ON);
      lastLedOn = now;
    } else {
      if (lastLedOn != -1 && now - lastLedOn > 300) {
        lastLedOn = -1;
        digitalWrite(LED_PIN, OFF);
      }
    }
  }
};

#endif
