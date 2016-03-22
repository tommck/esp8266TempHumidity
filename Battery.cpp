#include "Battery.h"
#include <Arduino.h>

Battery::Battery(int pin){
  _pin = pin;
}

void Battery::ReadLevels(int numReadings, float* readings) {
  for (int i=0; i < numReadings; i++) {
    // TODO: led.Toggle();
    
    int level = analogRead(_pin);

    readings[i] = level;

    yield();
  }
}  
