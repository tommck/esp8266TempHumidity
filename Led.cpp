#include "Led.h"
#include <Arduino.h>

Led::Led(int pin){
  _pin = pin;
  _ledState = LOW;
  
  pinMode(pin, OUTPUT); // Blue LED pin
}

void Led::OnOff(bool onOff) {
  if (onOff) 
  {      
    _ledState = HIGH;
  }
  else
  {
    _ledState = LOW;
  }
  digitalWrite(_pin, _ledState);
}  

void Led::Toggle() {
  OnOff(_ledState == LOW);
}

