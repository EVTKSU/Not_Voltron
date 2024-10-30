import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from sensor_msgs.msg import Imu, LaserScan, Image
import logging
import psutil

logging.basicConfig(filename='system_health.log', level=logging.INFO) #configure logging file

class MonitoringNode(Node):
    def __init__(self):
        super().__init__('monitoring_node')
        
        self.imu_subscription = self.create_subscription( #subscriptions
            Imu,
            'imu_data',
            self.imu_callback,
            10) #queue
        
        self.lidar_subscription = self.create_subscription(
            LaserScan,
            'lidar_data',
            self.lidar_callback,
            10)
        
        self.oak_subscription = self.create_subscription(
            Image,
            'oak_d_image',
            self.oak_callback,
            10)
        
        self.control_subscription = self.create_subscription(
            String,
            'control_status',
            self.control_callback,
            10)

        self.system_status_publisher = self.create_publisher(
            String, 
            'system_health_status', 
            10)
        
        self.critical_issue = False
        self.timer = self.create_timer(5.0, self.log_system_performance)

    def imu_callback(self, msg):
        print(f"Received IMU data: Orientation={msg.orientation}, Angular velocity={msg.angular_velocity}, Linear acceleration={msg.linear_acceleration}")
        logging.info(f"IMU data: Orientation={msg.orientation}, Angular velocity={msg.angular_velocity}, Linear acceleration={msg.linear_acceleration}")

    def lidar_callback(self, msg):
        print(f"Received LIDAR data: Ranges={msg.ranges}")
        logging.info(f"LIDAR data: Ranges={msg.ranges}")

    def oak_callback(self, msg):
        print(f"Received OAK-D Image data at time: {msg.header.stamp}")
        logging.info(f"OAK-D Image data received at time: {msg.header.stamp}") #maybe more here?

    def control_callback(self, msg):
        print(f"Received control response: {msg.data}")
        logging.info(f"Control response: {msg.data}")
        if msg.data == "missed":
            logging.error("Missed control command")
            self.send_alert("Missed control command")

    def send_alert(self, message):
        self.critical_issue = True
        alert_msg = String()
        alert_msg.data = f"Critical issue: {message}"
        self.system_status_publisher.publish(alert_msg)
        print(alert_msg.data)

    def log_system_performance(self):
        cpu_usage = psutil.cpu_percent()
        memory_info = psutil.virtual_memory()
        logging.info(f"System performance: CPU usage={cpu_usage}%, Memory usage={memory_info.percent}%")

def main(args=None):
    rclpy.init
    monitoring_node = MonitoringNode()
    rclpy.spin(monitoring_node)
    monitoring_node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()