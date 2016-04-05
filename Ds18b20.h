#include <OneWire.h>
#include <DallasTemperature.h>

class Ds18b20
{
    private: 
      OneWire* _oneWire;
      DallasTemperature* _sensors;
      
    public:
    Ds18b20(int pin);
    ~Ds18b20();

    void ReadTemp(int numReadings, float* temps);    

    void Close();
};
