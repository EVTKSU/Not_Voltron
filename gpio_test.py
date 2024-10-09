# -----------------/ Module Imports \--------------------- #

from time import perf_counter

import board
import digitalio

import adafruit_lsm9ds1

from imu import IMU

# -------------------------------------------------------- #



# -----------------/ General Notes \---------------------- #
"""
Gyroscope is measured in degrees per second
Accelerometer is measured in meters per second squared
Magnetometer is measured in gauss 
Temperature is measured in celcius 
"""
# -------------------------------------------------------- #


# ----------------/ Variables & Pins \-------------------- #

imu = IMU()

# -------------------------------------------------------- #



# ------------------/ General Methods \------------------- #

def clear_terminal() -> None:
    """Clears any previous terminal output
    """

    print('\033[H\033[2J')

# -------------------------------------------------------- #



# -----------------/ Main Code Section \------------------ #

def main() -> None:
    imu.set_gyro_range(adafruit_lsm9ds1.GYROSCALE_500DPS)

    for _ in range(1000):
        imu.update_sensor_values()
        vector = imu.velocity
        print(f'({vector[0]:0.3f}, {vector[1]:0.3f}, {vector[2]:0.3f})')
    


if __name__ == '__main__':
    clear_terminal()
    main()

# -------------------------------------------------------- #