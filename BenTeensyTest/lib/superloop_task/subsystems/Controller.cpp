#include "Controller.h"

#include <HAL.h>
#include <math.h>

const int CALIBRATE_BUTTON = 26;
const int CALIBRATE_RESET_TOGGLE = 12;

Controller::Controller(SBUS* x8r)
    : x8r_(x8r), channels_{0.0}, failSafe_(false), lostFrame_(false) {}

void Controller::init() {
  x8r_->begin();
  hal_pinMode(CALIBRATE_BUTTON, INPUT);
  hal_pinMode(CALIBRATE_RESET_TOGGLE, INPUT);
  last_success_millis_ = hal_millis();
}

void Controller::loop() {
  uint32_t cur_time = hal_millis();
  if (cur_time >= last_success_millis_ + 2000) {
    timed_out_ = true;
  } else {
    timed_out_ = false;
    // hal_printf("cur_time: %d\n", cur_time);
    // hal_printf("last_success_millis_: %d\n", last_success_millis_);
    // hal_printf("reads_since_: %d\n", reads_since_last_success_);
  }
}

bool Controller::get_timed_out() { return timed_out_; }

bool Controller::read() {
  bool success = x8r_->readCal(channels_, &failSafe_, &lostFrame_);
  if (!success) return false;

  if (!lostFrame_) {
    reads_since_last_success_ = 0;
    last_success_millis_ = hal_millis();
  } else {
    reads_since_last_success_++;
  }

  return true;
}

// NOTE: when the deadman is pulled on the master controller, it loses the
// ability to control the arm and calibrate switches!!!

// shared
// channel 1 = throttle [-1.0, 1.0]
// channel 2 = steering [-1.0, 1.0]
// channel 3 = mux {-1.0, 0} (-1 = RC, 0 = autonomous)
// channel 4 = deadman {-1.0, 1.0} (-1 = deadman released; 1 = deadman pulled)
// -> released = stop kart; pulled = go kart

// master controller only
// channel 6 = arm switch {0, 1} (0 =  not arming, 1 = arming)
// channel 7 = calibrate switch {0, 1} (0 = not calibrating, 1 = calibrating)

float Controller::get_throttle() {
  float throttle = channels_[1];
  // NOTE: when the deadman is pulled on the master controller, it loses the
  // ability to control the arm and calibrate switches!!!
  if (fabsf(throttle) < 0.01) {
    // deadband
    return 0.0;
  }

  return throttle;
}

float Controller::get_steering() { return channels_[3]; }

bool Controller::get_deadman_held() { return (channels_[4] > -0.3); }

bool Controller::get_armed() {
  return (channels_[8]) > 0.5;  // we need to do this because the value we get
                                // from the controller is exact and casting to
                                // int doesnt round the normal way
}

bool Controller::get_auto_mux() { return (channels_[7]) > -0.5; }

bool Controller::get_calibrate() { return (channels_[5]) > 0.5; }
