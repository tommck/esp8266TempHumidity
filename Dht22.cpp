#include "Dht22.h"
#include "Utils.h"

#define DHTTYPE DHT22

Dht22::Dht22(int pin) 
{
    // initialze the DHT-22
    _dht = new DHT(pin, DHTTYPE, 11); // 11 works fine for ESP8266
}

Dht22::~Dht22() {
  delete _dht;
  _dht = NULL;
}

void Dht22::ReadTempAndHumidity(int numReadings, float* temps, float* hums) 
{
    for (int i=0; i < numReadings; i++) 
    {
      float temp = _dht->readTemperature(true);     // Fahrenheit
      if (isnan(temp)) {
        temp = -1; // TODO: adjust code to ignore -1s
      }
      float humidity = _dht->readHumidity();        // percent
      if (isnan(humidity)) {
        humidity = -1; // TODO: adjust code to ignore -1s
      }
      Serial.print(F("Temp/Hum: "));
      Serial.print(temp);
      Serial.print(F("/"));
      Serial.println(humidity);
  
      temps[i] = temp;
      hums[i] = humidity;
  
      if (i < numReadings-1) 
      {
        Utils::Delay(2000); // mandatory between reads
      }
      yield();
    }
}    
