#ifndef StateMachine_h
#define StateMachine_h

#include "subsystems/Controller.h"
#include "subsystems/Drivetrain.h"
#include "subsystems/EStop.h"

class StateMachine {
public:
  enum State {
    Unarmed = 1,
    Idle = 2,
    RcControl = 3,
    AutoControl = 4,
    EmergencyRc = 5,
    EmergencyAuto = 6,
  };

  static constexpr const char* StateToString(State s) {
    switch (s) {
      case State::Unarmed:
        return "Unarmed";
      case State::Idle:
        return "Idle";
      case State::RcControl:
        return "RcControl";
      case State::AutoControl:
        return "AutoControl";
      case State::EmergencyRc:
        return "EmergencyRc";
      case State::EmergencyAuto:
        return "EmergencyAuto";
      default:
        return "Unknown State";
    }
  }

  State _state = State::Unarmed;
  uint32_t last_print_time = 0;
  uint32_t last_msg_recv_time = 0;
  float test;

  Drivetrain* drivetrain;
  Controller* rc;
  EStop* estop;

  StateMachine(Drivetrain* drivetrain, Controller* rc, EStop* estop);

  void init();
  void loop();
};

#endif
