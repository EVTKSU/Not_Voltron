#include "StateMachine.h"

#include <HAL.h>
#include <shared_data.h>
#include <util.h>

#include <evt_time.hpp>

uint32_t last_deadman_held_time = 0;

StateMachine::StateMachine(Drivetrain *drivetrain, Controller *rc, EStop *estop)
    : drivetrain(drivetrain), rc(rc), estop(estop) {}

void StateMachine::init() {
  drivetrain->init();
  rc->init();
  estop->init();
}

void StateMachine::loop() {
  uint32_t cur_millis = hal_millis();

  if (rc->get_deadman_held()) {
    last_deadman_held_time = millis();
  }

  bool deadman_L = false;
  if (cur_millis - last_deadman_held_time > 500) {
    deadman_L = true;
  }

  switch (_state) {
    case State::Unarmed: {
      if (!rc->read()) break;
      estop->release_estop();

      if (rc->get_calibrate()) {
        drivetrain->arm_odrive_full();
        _state = State::Idle;
      } else if (rc->get_armed()) {
        drivetrain->arm_odrive();
        _state = State::Idle;
      }

    } break;
    case State::Idle: {
      if (!rc->read()) break;

      estop->hold_estop();

      float drive_current = 0.0;
      float steer_rots = Drivetrain::rc_steering_to_rotations(rc->get_steering());
      drivetrain->drive(drive_current, steer_rots);
      shared_data_commanded_rpm = 0.0;
      shared_data_commanded_current = 0.0;
      shared_data_commanded_steer = steer_rots;

      if (rc->get_deadman_held()) {
        _state = State::RcControl;
      }

    } break;
    case State::RcControl: {
      if (!rc->read()) break;

      estop->hold_estop();

      float drive_current = Drivetrain::rc_throttle_to_current(rc->get_throttle());
      float steer_rots = Drivetrain::rc_steering_to_rotations(rc->get_steering());
      drivetrain->drive(drive_current, steer_rots);
      shared_data_commanded_rpm = 0.0;
      shared_data_commanded_current = drive_current;
      shared_data_commanded_steer = steer_rots;

      if (rc->get_auto_mux()) {
        _state = State::AutoControl;
      }
      if (false) {
        _state = State::EmergencyRc;
      }

    } break;
    case State::AutoControl: {
      if (!rc->read()) break;

      estop->hold_estop();

      if (!rc->get_auto_mux()) {
        _state = State::RcControl;
      }
      if (false) {
        _state = State::EmergencyAuto;
      }

      // We haven't recieved a microros message in over a second
      // Check for underflow (last_msg_recv_time can be updated in the other
      // task after cur_millis)
      if (cur_millis > shared_last_msg_recv_time &&
          cur_millis - shared_last_msg_recv_time > 400) {
        _state = State::EmergencyAuto;
      }

      if(shared_input_rpm == -1) {
        drivetrain->drive(shared_input_current, shared_input_steer);
        shared_data_commanded_current = shared_input_current;
        shared_data_commanded_steer = shared_input_steer;
        shared_data_commanded_rpm = 0;
      }
      else {
        drivetrain->drive_rpm(shared_input_rpm, shared_input_steer);
        shared_data_commanded_rpm = shared_input_rpm;
        shared_data_commanded_steer = shared_input_steer;
        shared_data_commanded_current = 0;
      }


    } break;
    case State::EmergencyRc: {
      rc->read();

      // Coast with steering
      float drive_current = 0.0;
      float steer_rots = Drivetrain::rc_steering_to_rotations(rc->get_steering());
      drivetrain->drive(0.0, steer_rots);
      shared_data_commanded_current = drive_current;
      shared_data_commanded_steer = steer_rots;

      estop->release_estop();

    } break;
    case State::EmergencyAuto: {
      // Coast
      drivetrain->drive(0.0, 0.0);
      shared_data_commanded_current = 0.0;
      shared_data_commanded_steer = 0.0;

      estop->release_estop();
    }  break;
  }

  drivetrain->loop();
  shared_data_state = _state;

  shared_data_reported_current = drivetrain->get_current();
  shared_data_reported_rpm = drivetrain->get_rpm();
  shared_data_reported_steer = drivetrain->get_steer();
  shared_data_drive_batt_voltage = drivetrain->get_drive_voltage(); //
  shared_data_steer_batt_voltage = drivetrain->get_steer_voltage(); //

  rc->loop();
 
  if (cur_millis - last_print_time > 50) {
    hal_println("=====");
    hal_printf("State: %s\n", StateToString(_state));
    hal_printf("Throttle: %f\n", rc->get_throttle());
    hal_printf("Steering: %f\n", rc->get_steering());
    hal_printf("Mux: %d\n", rc->get_auto_mux());
    hal_printf("Deadman: %d\n", rc->get_deadman_held());
    hal_printf("Armed: %d\n", rc->get_armed());
    hal_printf("Calibrate: %d\n", rc->get_calibrate());
    hal_printf("Timed Out: %d\n", rc->get_timed_out());
    hal_printf("Input Current: %f\n", shared_input_current);
    hal_printf("Input Steer: %f\n", shared_input_steer);
    hal_printf("Reported Current: %f\n", shared_data_reported_current);
    hal_printf("Reported Drive Voltage: %f\n", shared_data_drive_batt_voltage);
    hal_printf("Reported Steer Voltage %f\n", shared_data_steer_batt_voltage);
    hal_printf("Reported Steer: %f\n", shared_data_reported_steer);
    hal_printf("Commanded RPM: %f\n",shared_data_commanded_rpm);
    hal_printf("Reported RPM: %f\n", shared_data_reported_rpm);
    hal_printf("Input Last: %d\n", shared_last_msg_recv_time);
    hal_printf("Millis: %d\n", cur_millis);

    evt_time_t utc_time = evt_time_now();
    hal_printf("UTC: %d ; %d\n", utc_time.seconds, utc_time.microseconds);
    hal_printf("Loop micros: %d\n", shared_last_loop_micros);
    hal_println("=====");
    last_print_time = cur_millis;
  }
}
