#pragma once
#include "ISensorReader.h"
#include <vector>

class BatterySensorReader: public ISensorReader {
  private:
    int _pin;
    
  public:
    BatterySensorReader();

    std::vector<Reading> Read(int numReadingsToAverage);
};


