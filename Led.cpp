#include "Led.h"
#include <Arduino.h>

Led::Led(int pin, bool invert) {
    _pin = pin;
    _invert = invert;
    pinMode(_pin, OUTPUT);
    
    // always initialize to Off
    _ledState = _invert ? HIGH : LOW; 
    digitalWrite(_pin, _ledState);
}

void Led::OnOff(bool onOff) {
    if (_invert) {
        onOff = !onOff;
    }
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

