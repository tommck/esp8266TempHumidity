#include "Utils.h"
#include <Arduino.h>
#include <QuickStats.h>

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

// global QuickStats for simple stats
QuickStats stats;

int Utils::normalizeReadings(float* readings, int length) {
  stats.bubbleSort(readings, length);

  // trims off the largest and smallest values (if possible) and averages the rest
  if (length > 2) {
    return (int)stats.average(&readings[1], length-2);  
  }
  return (int)stats.average(readings, length);  
}

int Utils::normalizeReadings(int* readings, int length) {
  float floatReadings[length];
  for (int i=0; i < length; ++i) {
    floatReadings[i] = (float)readings[i];
  }

  return Utils::normalizeReadings(floatReadings, length);
}

