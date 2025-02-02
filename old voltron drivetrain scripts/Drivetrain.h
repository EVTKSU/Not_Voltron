#ifndef Drivetrain_h
#define Drivetrain_h

#include <ODriveTeensyCAN.h>
#include <util.h>

class Drivetrain {
private:
  ODriveTeensyCAN* odrive_;

  void steer(float s_val);
  void calibrate_odrive();
  void reset_odrive();

public:
  // the max steering rotations (motor rotations)
  static const float MAX_STEERING;
  // Max current to the vesc
  static const float MAX_VESC_CURRENT;
  // ODrive CAN node id (axis id)
  static const int AXIS_STEERING;
  // VESC CAN node id
  static const int VESC_ID;

  inline static float rc_throttle_to_current(float rc_throttle) {
    return rc_throttle * MAX_VESC_CURRENT;
  }
  
  inline static float rc_steering_to_rotations(float rc_steering) {
    return -rc_steering * MAX_STEERING;
  }

  Drivetrain(ODriveTeensyCAN* odrive);

  void init();
  void arm_odrive();
  void arm_odrive_full();

  void drive(float drive_current, float steering_pos);
  void drive_rpm(float drive_rpm, float steering_pos);

  void loop();

  float get_rpm();
  float get_current();
  float get_steer();
  float get_drive_voltage(); //
  float get_steer_voltage();
};

#endif
