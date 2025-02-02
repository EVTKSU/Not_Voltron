#include "EStop.h"

#include <HAL.h>

const int EStop::RELAY_IN = 15;  // pin for contacter relay

void EStop::init() {
  // We only want to arm the estop when we're in control.
  // The clamp stays on until we're in control mode.
  hal_pinMode(RELAY_IN, OUTPUT);
  release_estop();
}

void EStop::release_estop() { hal_digitalWrite(RELAY_IN, LOW); }

void EStop::hold_estop() { hal_digitalWrite(RELAY_IN, HIGH); }
