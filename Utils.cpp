#include "Utils.h"
#include <Arduino.h>

void Utils::Delay(int ms) 
{
  int msRemaining = ms;
  while(msRemaining > 0) {
    delay(100 < msRemaining ? 100 : msRemaining);
    yield();
    msRemaining -= 100;
  }
/*  
 This is supposed to prevent problems with watchdog timer resetting - clueless
    for(int i=1; i != ms;i++) 
    {
          delay(1);
          if(i%100 == 0) {
                  ESP.wdtFeed(); 
                  yield();
          }
    }
  */
}

