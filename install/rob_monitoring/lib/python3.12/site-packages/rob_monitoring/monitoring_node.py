import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from sensor_msgs.msg import Imu, LaserScan, Image  # Import Image for OAK-D vision data
import logging
import psutil

# Initialize logging system to log messages to system_health.log at info level
logging.basicConfig(filename='system_health.log', level=logging.INFO)

class MonitoringNode(Node):
    def __init__(self):
        super().__init__('monitoring_node')
        
        # Subscription for IMU data
        self.imu_subscription = self.create_subscription(
            Imu,
            'imu_data',  # Replace with the actual topic name for IMU
            self.imu_callback,
            10
        )
        
        # Subscription for LIDAR data
        self.lidar_subscription = self.create_subscription(
            LaserScan,
            'lidar_data',  # Replace with the actual topic name for LIDAR
            self.lidar_callback,
            10
        )
        
        # Subscription for OAK-D vision data (Image type)
        self.oak_subscription = self.create_subscription(
            Image,
            'oak_d_image',  # Replace with the actual topic name for OAK-D
            self.oak_callback,
            10
        )

        # Subscription for control status
        self.control_subscription = self.create_subscription(
            String,
            'control_status',
            self.control_callback,
            10
        )

        # Publisher for system health alerts
        self.system_status_publisher = self.create_publisher(String, 'system_health_status', 10)
        
        self.critical_issue_detected = False
        self.timer = self.create_timer(5.0, self.log_system_performance)

    def imu_callback(self, msg):
        logging.info(f"IMU data: Orientation={msg.orientation}, Angular velocity={msg.angular_velocity}, Linear acceleration={msg.linear_acceleration}")
        # Add error checking for IMU data if needed

    def lidar_callback(self, msg):
        logging.info(f"LIDAR data: Ranges={msg.ranges}")
        # Add error checking for LIDAR data if needed

    def oak_callback(self, msg):
        logging.info(f"OAK-D Image data received at time: {msg.header.stamp}")
        # You could add additional logging or processing for vision data here, such as checking for data consistency

    def control_callback(self, msg):
        logging.info(f"Control response: {msg.data}")
        if msg.data == "missed":
            logging.error("Missed control command")
            self.send_alert("Missed control command")

    def send_alert(self, message):
        self.critical_issue_detected = True
        alert_msg = String()
        alert_msg.data = f"Critical issue: {message}"
        self.system_status_publisher.publish(alert_msg)
        print(alert_msg.data)

    def log_system_performance(self):
        cpu_usage = psutil.cpu_percent()
        memory_info = psutil.virtual_memory()
        logging.info(f"System performance: CPU usage={cpu_usage}%, Memory usage={memory_info.percent}%")

def main(args=None):
    rclpy.init(args=args)
    monitoring_node = MonitoringNode()
    rclpy.spin(monitoring_node)
    monitoring_node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
