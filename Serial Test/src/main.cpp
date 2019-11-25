#include <Arduino.h>

#define RX1 4
#define TX1 2

#define RX2 16
#define TX2 17

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RX1, TX1);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  // put your setup code here, to run once:
}

void loop() {
  if(Serial.available())
  {
    Serial2.println(Serial.readStringUntil('&'));
  }
  if (Serial2.available())
    {
      Serial.println(Serial2.readStringUntil('&'));
    }
  // put your main code here, to run repeatedly:
}