#include <DHT.h>

class Dht22 
{
    private: 
    DHT* _dht;
      
    public:
    Dht22(int pin);
    ~Dht22();

    void ReadTempAndHumidity(int numReadings, float* temps, float* hums);    

    void Close();
};
