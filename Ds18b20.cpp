#include <DallasTemperature.h>

#include "Ds18b20.h"
#include "Utils.h"

Ds18b20::Ds18b20(int pin) 
{
    // initialze the protocol
    _oneWire = new OneWire(pin);
    _sensors = new DallasTemperature(_oneWire);
    _sensors->begin();
}

Ds18b20::~Ds18b20() {
  delete _oneWire;
  _oneWire = NULL;
}

void Ds18b20::ReadTemp(int numReadings, float* temps)
{
    for (int i = 0; i < numReadings; i++)
    {
        _sensors->requestTemperatures();
        temps[i] = _sensors->getTempFByIndex(0);
        Serial.printf("Ds18b20 Temp: %d\n", (int)temps[i]);
        yield();
    }
}
