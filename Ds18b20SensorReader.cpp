#include "ISensorReader.h"
#include "Ds18b20SensorReader.h"

Ds18b20SensorReader::Ds18b20SensorReader() : _oneWire(Config::ds18b20Pin), _sensors(&_oneWire)
{
  _sensors.begin();
}

std::vector<Reading> Ds18b20SensorReader::Read(int numReadingsToAverage) 
{
  float temps[numReadingsToAverage];
  for (int i = 0; i < numReadingsToAverage; i++)
  {
      _sensors.requestTemperatures();
      temps[i] = _sensors.getTempFByIndex(0);
      //Serial.printf("Ds18b20 Temp: %d\n", (int)temps[i]);
      yield();
  }
  
  return std::vector<Reading>({
    Reading(ReadingType::SoilTemperature, Utils::normalizeReadings(temps, numReadingsToAverage))
  });
}

