#pragma once
#include "ISensorReader.h"
#include <vector>
#include <DHT.h>

class Dht22SensorReader : public ISensorReader {
  private: 
    DHT _dht;
    
  public:
    Dht22SensorReader();

    std::vector<Reading> Read(int numReadingsToAverage);
};


