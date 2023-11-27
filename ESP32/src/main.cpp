#include <Arduino.h>
#include "Access_Point.h"

void setup() {
  Serial.begin(9600);
  Init_AP();
}

void loop() {
  CAN_Mockup();
}

