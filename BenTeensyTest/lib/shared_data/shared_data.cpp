#include "shared_data.h"

volatile uint32_t shared_last_msg_recv_time;  // milliseconds

volatile int shared_data_state;  // from enum

// volatile float shared_data_brake_torque; // N/m
volatile float shared_data_commanded_current;  // amps
volatile float shared_data_commanded_steer;    // revolutions of steering motor
volatile float shared_data_commanded_rpm;

volatile float shared_data_reported_current;  // amps
volatile float shared_data_reported_steer;    // revolutions of steering motor
volatile float shared_data_reported_rpm;      // drive motor rpm
volatile float shared_data_drive_batt_voltage; // drive battery voltage //

volatile float shared_input_current;  // amps
volatile float shared_input_rpm;
volatile float shared_input_steer;    // revolutions of steering motor
volatile float shared_data_steer_batt_voltage; //

volatile uint32_t shared_last_loop_micros;  // microseconds for loop
