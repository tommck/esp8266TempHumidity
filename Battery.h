
class Battery {
  private:
  int _pin;

  public:
  
  Battery(int pin);

  void ReadLevels(int numReadings, float* readings);
};

