#include "pidSystem.hpp"

using namespace std::chrono;
using std::abs;


/// @brief Defines a PID controller that can be used for anything
/// @param KP Proportional gain 
/// @param KI Integral gain 
/// @param KD Derivative gain
/// @note Defines the value range to be [-1, 1] by default to be used as a percentage
PidSystem::PidSystem(double KP, double KI, double KD) {
  kP = KP;
  kD = KD;
  kI = KI;

  minOutput = -1;
  maxOutput = 1;
}


/// @brief Defines a PID controller that has a range of acceptable values 
/// @param KP Propotional gain
/// @param KI Integral gain 
/// @param KD Derivative gain
/// @param minValue Minium value for the PID controller output
/// @param maxValue Maximum value for the PID controller output
PidSystem::PidSystem(double KP, double KI, double KD, double minValue, double maxValue) {
  kP = KP;
  kD = KD;
  kI = KI;

  minOutput = minValue;
  maxOutput = maxValue;
}


/// @brief Calculates the value of motor power for the given setpoint
/// @param position The current position of the robot
/// @param setpoint The setpoint for the intented position 
/// @return The calculated value for the motors
double PidSystem::calculate(double position, double setpoint) {
  error = setpoint - position; 

  if (abs(error) < kIntegrationLimit) {
    errorSum += error * deltaT;
  }
  
  errorRate = (error - lastError) / deltaT;

  lastTimestamp += deltaT;
  lastError = error;

  output = (error * kP) + (errorSum * kI) + (errorRate * kD);

  return constrain<double>(output, minOutput, maxOutput);
} 


/// @brief Sets the value range for the PID controller velocity constraints 
/// @param minVal Minimum velocity value 
/// @param maxVal Maximum velocity value 
void PidSystem::setValueRange(double minVal, double maxVal) {
  minOutput = minVal;
  maxOutput = maxVal;
}


/// @brief Returns a value between a certain range 
/// @tparam T Data type for the return value
/// @param x Value to be constrained 
/// @param min Minimum value 
/// @param max Maximum value 
/// @return Constrained value
template<class T>
inline T PidSystem::constrain(T x, T min, T max) {
  return x >= min ? x <= max ? x : max : min;
}


/// @brief Returns whether or not the PID system is done running
/// @return Whether or not the robot is moving slow enough to be considered stopped
bool PidSystem::isFinished() {
  return errorRate <= finishedValue;
}


/// @brief Sets the value to determine if the PID system is finished 
/// @param value New value for the finished condition 
void PidSystem::setFinishedValue(double value) {
  finishedValue = value;
}


/// @brief Gets the delta t value
/// @return Change in time between each loop in milliseconds
double PidSystem::getDeltaT() {
  return deltaT;
}