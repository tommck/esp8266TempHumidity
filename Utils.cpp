#include "Utils.h"
#include <Arduino.h>

void Utils::Delay(int ms) 
{
  int msRemaining = ms;
  while(msRemaining > 0) {
    delay(100 < msRemaining ? 100 : msRemaining);
    ESP.wdtFeed(); 
    yield();
    msRemaining -= 100;
  }
}
