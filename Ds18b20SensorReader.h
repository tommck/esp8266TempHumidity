#pragma once
#include "ISensorReader.h"
#include "Config.h"
#include <vector>
#include <OneWire.h>
#include <DallasTemperature.h>
 
class Ds18b20SensorReader : public ISensorReader {
  private: 
    OneWire _oneWire;
    DallasTemperature _sensors;
    
  public:
    Ds18b20SensorReader();

    std::vector<Reading> Read(int numReadingsToAverage);
};

