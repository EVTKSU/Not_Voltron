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

loss_file: str = 'COMP_2025_TESTING/lib/IMU_Sensor/loss_loss.csv'

# -------------------------------------------------------- #



# ------------------/ General Methods \------------------- #

def clear_terminal() -> None:
    """Clears any previous terminal output
    """

    print('\033c', end='')


def average(values: list) -> float:
    """Averages the values in a list

    Args:
        values (list): List of float values to average

    Returns:
        float: The average value of the list
    """

    return sum(values) / len(values)


def total_loss(actual: list, const_predicted: float) -> float:
    """Returns the value of the total loss of the sensor value

    Returns:
        float: Loss value
    """

    loss: float = 0

    for i in range(len(actual)):
        loss += (actual[i] - const_predicted) ** 2

    return loss
    

def write_loss_loss(file_name: str, values: dict) -> None:
    new_row: list = [list(values.values())]

    with open(file_name, 'a', newline='') as csvFile:
        writer = csv.writer(csvFile)
        writer.writerows(new_row)
        csvFile.close()


# -------------------------------------------------------- #



# -----------------/ Main Code Section \------------------ #

def main() -> None:
    imu.set_accel_range(adafruit_lsm9ds1.ACCELRANGE_4G)
    mpu.set_accel_range(adafruit_mpu6050.Range.RANGE_4_G)

    imu_loss_loss: list = []
    mpu_loss_loss: list = []

    loss_loss: dict = {'Least Loss': '', 'LSM9DS1 Loss': 0, 'MPU6050 Loss': 0}
    imu_loss_average: float = 0
    mpu_loss_average: float = 0

    for i in range(4):
        LSM9DS1: list = []
        MPU6050: list = []

        for _ in range(1000):
            imu.update_sensor_values()
            LSM9DS1.append(imu.get_accel_values()[IMU.Z_AXIS])
        

        for _ in range(1000):
            mpu.update_sensor_values()
            MPU6050.append(mpu.get_accel_values()[MPU.Z_AXIS])
            

        imu_average: float = average(LSM9DS1)
        mpu_average: float = average(MPU6050)
        

        imu_loss: float = total_loss(LSM9DS1, imu_average)
        mpu_loss: float = total_loss(MPU6050, mpu_average)
        
        imu_loss_loss.append(round(imu_loss, 3))
        mpu_loss_loss.append(round(mpu_loss, 3))

        imu_loss_average: float = average(imu_loss_loss)
        mpu_loss_average: float = average(mpu_loss_loss)

        loss_loss['Least Loss'] = ('MPU6050', 'LSM9DS1') [mpu_loss > imu_loss]
        loss_loss['LSM9DS1 Loss'] = round(imu_loss, 3)
        loss_loss['MPU6050 Loss'] = round(mpu_loss, 3)

        write_loss_loss(loss_file, loss_loss)

        clear_terminal()
        print(f'Iteration {i + 1} finsihed')


    clear_terminal()

    print(f'Total Loss Values:')
    print(f'  - LSM9DS1: {round(average(imu_loss_loss), 3)}')
    print(f'    > {imu_loss_loss}')
    print(f'  - MPU6050: {round(average(mpu_loss_loss), 3)}')
    print(f'    > {mpu_loss_loss}')
    print(f'Least Loss: {('MPU6050', 'LSM9DS1') [mpu_loss > imu_loss]}')

    


if __name__ == '__main__':
    clear_terminal()
    main()

# -------------------------------------------------------- #