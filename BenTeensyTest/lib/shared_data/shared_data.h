#ifndef shared_data_h
#define shared_data_h
#include <stdint.h>

// Writes to an aligned 32 bit variable are atomic
//  and we only have one core, so no lock needed.

extern volatile uint32_t shared_last_msg_recv_time;  // milliseconds

extern volatile int shared_data_state;  // from enum

// extern volatile float shared_data_brake_torque; // N/m
extern volatile float shared_data_commanded_current;  // amps
extern volatile float shared_data_commanded_rpm;
extern volatile float
    shared_data_commanded_steer;  // revolutions of steering motor

extern volatile float shared_data_reported_current;  // amps
extern volatile float
    shared_data_reported_steer;  // revolutions of steering motor
extern volatile float shared_data_reported_rpm;  // drive motor rpm
extern volatile float shared_data_drive_batt_voltage; //

extern volatile float shared_input_current;  // amps
extern volatile float shared_input_rpm;
extern volatile float shared_input_steer;    // revolutions of steering motor
extern volatile float shared_data_steer_batt_voltage; //

extern volatile uint32_t shared_last_loop_micros;  // microseconds for loop

#endif
