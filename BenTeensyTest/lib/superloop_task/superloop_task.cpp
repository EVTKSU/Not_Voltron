#include "superloop_task.h"

#include <HAL.h>
#include <ODriveTeensyCAN.h>
#include <SBUS.h>
#include <util.h>
#include <vesc_can.h>

#include "StateMachine.h"
#include "subsystems/Controller.h"
#include "subsystems/Drivetrain.h"
#include "subsystems/EStop.h"

void superloop_task(void* /*pvParameters*/) {
  SBUS x8r(Serial8);
  ODriveTeensyCAN odrive;

  Controller rc(&x8r);
  Drivetrain drivetrain(&odrive);
  EStop estop;

  StateMachine state_machine(&drivetrain, &rc, &estop);
  state_machine.init();

  while (true) {
    state_machine.loop();
  }
}
