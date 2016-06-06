#include "Dht22SensorReader.h"
#include <vector>
#include <DHT.h>
#include "Config.h"

Dht22SensorReader::Dht22SensorReader() : _dht(Config::dhtPin, Config::dhtType, 11) // 11 works fine for ESP8266
{
}

std::vector<Reading> Dht22SensorReader::Read(int numReadingsToAverage) {

  float temps[numReadingsToAverage];
  float hums[numReadingsToAverage];
  
  for (int i=0; i < numReadingsToAverage; i++) 
  {
    float temp = _dht.readTemperature(true);     // Fahrenheit
    if (isnan(temp)) {
      temp = -1; // TODO: adjust code to ignore -1s
    }
    float humidity = _dht.readHumidity();        // percent
    if (isnan(humidity)) {
      humidity = -1; // TODO: adjust code to ignore -1s
    }
    //Serial.print(F("Temp/Hum: "));
    //Serial.print(temp);
    //Serial.print(F("/"));
    //Serial.println(humidity);

    temps[i] = temp;
    hums[i] = humidity;

    if (i < numReadingsToAverage-1) 
    {
      Utils::Delay(2000); // mandatory between reads
    }
    yield();
  }

  return std::vector<Reading>({
    Reading(ReadingType::AmbientTemperature, Utils::normalizeReadings(temps, numReadingsToAverage)),
    Reading(ReadingType::AmbientHumidity, Utils::normalizeReadings(hums, numReadingsToAverage))
  });
}

