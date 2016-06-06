#pragma once
#include "Config.h"
#include "ReadingType.h"
#include "Utils.h"
#include <vector>

enum SensorType {
  Dht22,
  Ds18b20,
  Battery
};

typedef struct ReadingStruct {
  ReadingStruct(ReadingType type, int val) {
    Type = type;
    Value = val;
  }
   ReadingType Type;
   int Value;
} Reading;

class ISensorReader {
  public:
    ISensorReader(){}
    virtual ~ISensorReader(){}
     virtual std::vector<Reading> Read(int numReadingsToAverage) = 0;
};

