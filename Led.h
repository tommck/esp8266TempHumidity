class Led {
  private:
  int _pin;
  int _ledState;
  bool _invert;

  public:
  
  Led(int pin, bool invert=false);

  void OnOff(bool onOff);
  void Toggle();
};

