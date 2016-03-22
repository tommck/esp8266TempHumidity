class Led {
  private:
  int _pin;
  int _ledState;

  public:
  
  Led(int pin);

  void OnOff(bool onOff);
  void Toggle();
};

