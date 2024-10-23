#ifndef PID_SYSTEM
#define PID_SYSTEM 

#include <chrono>
#include <cmath>

/// @brief Class for use as a PID controller 
class PidSystem {
  public:
    PidSystem(double KP, double KI = 0, double KD = 0);
    PidSystem(double KP, double KI, double KD, double minValue, double maxValue);

    double calculate(double setpoint, double position);

    void setValueRange(double minVal, double maxVal);

    template <class T>
    inline static T constrain(T x, T min, T max);

    bool isFinished();
    void setFinishedValue(double value);

    static double getDeltaT();

  private:
    const double kIntegrationLimit = 6; // Limits the integral term to only within 12 inches of the setpoint

    double finishedValue = 0.1; // Value used to determine whether or not the PID system in complete

    double kP, kI, kD; 
    double error, errorRate, errorSum;
    static const double deltaT = 10; // Change in time in milliseconds
    double lastError, lastTimestamp;

    double output, minOutput, maxOutput;

    std::chrono::high_resolution_clock time;
};

#endif // PID_SYSTEM