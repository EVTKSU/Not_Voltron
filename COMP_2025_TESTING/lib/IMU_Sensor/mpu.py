# -----------------/ Module Imports \--------------------- #

import time
import board
import adafruit_mpu6050

# -------------------------------------------------------- #



# -----------------/ General Notes \---------------------- #
"""
The I2C address of the MPU6050 is 0x68 by default
"""
# -------------------------------------------------------- #



class MPU:
    """Class for data readings of a MPU6050 6DoF IMU sensor 
    """

    X_AXIS: int = 0
    Y_AXIS: int = 1
    Z_AXIS: int = 2


    def __init__(self) -> None:
        self.i2c = board.I2C()
        self.sensor = adafruit_mpu6050.MPU6050(self.i2c)

        self.accel_x, self.accel_y, self.accel_z = self.sensor.acceleration
        self.accel: tuple = (self.accel_x, self.accel_y, self.accel_z)

        self.gyro_x, self.gyro_y, self.gyro_z = self.sensor.gyro
        self.gyro = (self.gyro_x, self.gyro_y, self.gyro_z)

        self.temp: float = self.sensor.temperature

        self.last_timestamp: float = time.perf_counter() # Previous timestamp in seconds
        self.delta_t: float = 0.0                        # Change in time in seconds

        self.velocity: list = [0.0, 0.0, 0.0]            # Velocity values
        self.position: list = [0.0, 0.0, 0.0]            # Position values


    def update_sensor_values(self) -> None:
        """ Updates the values of the gyro, accelerometer, and magnetometer
        """

        # Updates sensor tuples 
        self.accel_x, self.accel_y, self.accel_z = self.sensor.acceleration
        self.accel: tuple = (self.accel_x, self.accel_y, self.accel_z)

        self.gyro_x, self.gyro_y, self.gyro_z = self.sensor.gyro
        self.gyro = (self.gyro_x, self.gyro_y, self.gyro_z)

        self.temp: float = self.sensor.temperature

        # Updates change in time
        self.delta_t = time.perf_counter() - self.last_timestamp

        # Updates position and velocity tuples
        for val in range(len(self.accel)):
            # self.velocity[val] += self.accel[val] * self.delta_t
            self.position[val] += (0.5  * self.accel[val]) * (self.delta_t ** 2)

        # Updates the last timestamp
        self.last_timestamp = time.perf_counter()

    

    def get_gyro_values(self) -> tuple:
        """Gets the orientation of the gyro

        Returns:
            tuple: Gyro position (x, y, z)
        """

        return self.gyro


    
    def get_accel_values(self) -> tuple:
        """Gets the values of the accelerometer

        Returns:
            tuple: Accelerometer values (x, y, z)
        """

        return self.accel



    def get_temperature(self) -> float:
        """Gets the temperature value in degrees celcius

        Returns:
            float: Temperature value
        """

        return self.temp



    def set_gyro_range(self, range) -> None:
        """Sets the gyroscope value range (deg/s)
        """

        self.sensor.gyro_range = range


    
    def set_accel_range(self, range) -> None:
        """Sets the accelerometer value range (m/s^2)
        """

        self.sensor.accelerometer_range = range