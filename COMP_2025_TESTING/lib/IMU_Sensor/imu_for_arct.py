# -----------------/ Module Imports \--------------------- #

from time import perf_counter, sleep

import board
import digitalio

import adafruit_lsm9ds1
import adafruit_mpu6050

import csv

from imu import IMU
from mpu import MPU

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

imu = IMU() # LSM9DS1 Sensor
mpu = MPU() # MPU6050 Sensor

refresh_rate: float = 0.25

# -------------------------------------------------------- #



# ------------------/ General Methods \------------------- #

def clear_terminal() -> None:
    """Clears any previous terminal output
    """

    print('\033c', end='')


# -------------------------------------------------------- #



# -----------------/ Main Code Section \------------------ #

def main() -> None:
    # Set the range of values for the IMU sensors to check
    imu.set_accel_range(adafruit_lsm9ds1.ACCELRANGE_4G)
    mpu.set_accel_range(adafruit_mpu6050.Range.RANGE_4_G)


    # Saves the LSM9DS1 data as a tuple of (x, y, z)
    imu_accel: tuple = ()
    imu_gyro: tuple = ()
    

    # Saves the MPU6050 data as a tuple of (x, y, z)
    mpu_accel: tuple = ()
    mpu_gyro: tuple = ()


    # Runs a while loop to get sensor data until a keyboard interrupt (Ctrl + C) 
    while True:
        try:
            # Clears the terminal of previous output
            clear_terminal()


            # Updates the sensor values 
            imu.update_sensor_values() # LSM9DS1 -- Accel, Gyro, Mag
            mpu.update_sensor_values() # MPU6050 -- Accel, Gyro


            # Updates data for later use
            imu_accel = imu.get_accel_values()
            imu_gyro = imu.get_gyro_values()

            mpu_accel = mpu.get_accel_values()
            mpu_gyro = mpu.get_gyro_values()


            # Prints the accel and gyro values of both sensors to the terminal
            print(f"Accel -- LSM9DS1: ({imu_accel[IMU.X_AXIS]:0.3f}, {imu_accel[IMU.Y_AXIS]:0.3f}, {imu_accel[IMU.Z_AXIS]:0.3f})")
            print(f"Gyro  -- LSM9DS1: ({imu_gyro[IMU.X_AXIS]:0.3f}, {imu_gyro[IMU.Y_AXIS]:0.3f}, {imu_gyro[IMU.Z_AXIS]:0.3f})\n")
            print(f"Accel -- MPU6050: ({mpu_accel[MPU.X_AXIS]:0.3f}, {mpu_accel[MPU.Y_AXIS]:0.3f}, {mpu_accel[MPU.Z_AXIS]:0.3f})")
            print(f"Gyro  -- MPU6050: ({mpu_gyro[MPU.X_AXIS]:0.3f}, {mpu_gyro[MPU.Y_AXIS]:0.3f}, {mpu_gyro[MPU.Z_AXIS]:0.3f})")


            # Waits a previously specified amount of time
            sleep(refresh_rate)
        except KeyboardInterrupt:
            break

    


if __name__ == '__main__':
    # Clears the terminal and runs the main code section
    clear_terminal()
    main()

# -------------------------------------------------------- #