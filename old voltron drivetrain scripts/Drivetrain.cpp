#include "Drivetrain.h"

#include <HAL.h>
#include <math.h>
#include <shared_data.h>
#include <util.h>
#include <vesc_can.h>

#include "elapsedMillis.h"

const int Drivetrain::AXIS_STEERING = 3;
const int Drivetrain::VESC_ID = 72;

const float Drivetrain::MAX_STEERING = 2.35;
const float Drivetrain::MAX_VESC_CURRENT = 500;

Drivetrain::Drivetrain(ODriveTeensyCAN* odrive) : odrive_(odrive) {}

void Drivetrain::init() {
  hal_pinMode(LED_BUILTIN, OUTPUT);
  vesc_can_set_motor_poles(VESC_ID, 8);
  vesc_can_begin();
}

void Drivetrain::arm_odrive() {
  hal_digitalWrite(LED_BUILTIN, HIGH);
  delay(300);
  hal_digitalWrite(LED_BUILTIN, LOW);
  delay(300);
  hal_digitalWrite(LED_BUILTIN, HIGH);

  hal_println("==== ARMING STEERING AXIS ====");

  odrive_->RunState(AXIS_STEERING,
                    ODriveTeensyCAN::AXIS_STATE_CLOSED_LOOP_CONTROL);
  while (odrive_->GetCurrentState(AXIS_STEERING) != ODriveTeensyCAN::AXIS_STATE_CLOSED_LOOP_CONTROL) {
    hal_printf("State: %d\n", odrive_->GetCurrentState(AXIS_STEERING));
    delay(100);
  }

  hal_println("==== ARMED STEERING AXIS ====");
  hal_digitalWrite(LED_BUILTIN, LOW);
}

void Drivetrain::calibrate_odrive() {
  Serial.println("Calibrating ODrive");
  hal_digitalWrite(LED_BUILTIN, LOW);

  delay(500);
  hal_digitalWrite(LED_BUILTIN, HIGH);
  hal_println("==== CALIBRATING STEERING AXIS ====");
  delay(200);
  hal_println("==== HOMING STEERING AXIS ====");
  odrive_->RunState(AXIS_STEERING, ODriveTeensyCAN::AXIS_STATE_HOMING);
  delay(100);
  while (odrive_->GetCurrentState(AXIS_STEERING) !=
         ODriveTeensyCAN::AXIS_STATE_IDLE) {
    hal_printf("Steering State: %d\n",
               odrive_->GetCurrentState(AXIS_STEERING));
    delay(100);
  
  }
  hal_println("==== HOMED STEERING AXIS ====");

  odrive_->RunState(AXIS_STEERING, ODriveTeensyCAN::AXIS_STATE_FULL_CALIBRATION_SEQUENCE);
  delay(100);
  while (odrive_->GetCurrentState(AXIS_STEERING) !=
         ODriveTeensyCAN::AXIS_STATE_IDLE) {
    hal_printf("Steering State: %d\n",
               odrive_->GetCurrentState(AXIS_STEERING));
    delay(100);
  
  }
  hal_println("==== CALIBRATED STEERING AXIS ====");
}

void Drivetrain::reset_odrive() {
  odrive_->ClearErrors(AXIS_STEERING);
  delay(100);
}

void Drivetrain::arm_odrive_full() {
  reset_odrive();
  hal_digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  // Calibrating steering and braking axises.
  calibrate_odrive();
  // Arming Odrive
  arm_odrive();
}

void Drivetrain::steer(float rotations) {
  if (rotations > MAX_STEERING){
    odrive_->SetPosition(AXIS_STEERING, MAX_STEERING);
  }
  else if (rotations < -MAX_STEERING) {
    odrive_->SetPosition(AXIS_STEERING, -MAX_STEERING);
  }
  else{
    odrive_->SetPosition(AXIS_STEERING, rotations);
  }
}

void Drivetrain::drive(float drive_current, float steering_pos) {
  steer(steering_pos);
  if (drive_current < 0.0) {
    vesc_can_set_current_brake(VESC_ID, fabsf(drive_current));
  } else {
    vesc_can_set_current(VESC_ID, drive_current);
  }
}

void Drivetrain::drive_rpm(float drive_rpm, float steering_pos) {
  steer(steering_pos);
  vesc_can_set_rpm(VESC_ID, drive_rpm);
}

static uint32_t last_odrive_request_time = 0;
void Drivetrain::loop() {
  vesc_can_read();
  odrive_->readAsyncMessages();

  uint32_t cur_time = millis();
  if (cur_time - last_odrive_request_time > 10) {
    //Serial.println("requested boi");
    //odrive_->RequestPositionEstimate(AXIS_STEERING);
    odrive_->RequestBusVoltageCurrent(AXIS_STEERING);
    last_odrive_request_time = cur_time;
    // Serial.println("Request Voltage");
  }
}

float Drivetrain::get_rpm() { return vesc_can_get_rpm(VESC_ID); }

float Drivetrain::get_current() { return vesc_can_get_current(VESC_ID); }

float Drivetrain::get_drive_voltage() { return vesc_can_get_voltage(VESC_ID); } //

float Drivetrain::get_steer() {
  return odrive_->GetLastPositionEstimate(AXIS_STEERING);
}

float Drivetrain::get_steer_voltage() {
  return odrive_->GetLastBusVoltage(AXIS_STEERING);
}
