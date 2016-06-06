#include "BatterySensorReader.h"
#include <vector>
#include "Config.h"
#include "Utils.h"

BatterySensorReader::BatterySensorReader() : _pin(Config::batteryPin)
{}

std::vector<Reading> BatterySensorReader::Read(int numReadingsToAverage)
{
  int readings[numReadingsToAverage];
  for (int i=0; i < numReadingsToAverage; i++) {
    
    int level = analogRead(_pin);

    readings[i] = level;

    yield();
  }

  return std::vector<Reading>({
    Reading(ReadingType::BatteryLevel, Utils::normalizeReadings(readings, numReadingsToAverage))
  });
}


