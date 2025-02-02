#ifndef EStop_h
#define EStop_h

class EStop {
private:
  static const int RELAY_IN;  // pin for emag relay

public:
  void init();

  void release_estop();
  void hold_estop();
};

#endif
