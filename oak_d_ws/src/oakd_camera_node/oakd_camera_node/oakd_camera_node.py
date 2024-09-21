import rclpy
from rclpy.node import Node
import depthai as dai
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import cv2

class OAKDCameraNode(Node):
    def __init__(self):
        super().__init__('oakd_camera_node')
        self.bridge = CvBridge()
        self.publisher_rgb = self.create_publisher(Image, 'camera/rgb', 10)
        self.publisher_depth = self.create_publisher(Image, 'camera/depth', 10)
        
        # Set up OAK-D pipeline
        self.pipeline = dai.Pipeline()
        self.cam_rgb = self.pipeline.createColorCamera()
        self.cam_rgb.setPreviewSize(640, 480)
        self.cam_rgb.setInterleaved(False)
        
        self.depth_camera = self.pipeline.createStereoDepth()
        self.depth_camera.setLeftRightCheck(True)
        
        xout_rgb = self.pipeline.createXLinkOut()
        xout_rgb.setStreamName("rgb")
        self.cam_rgb.preview.link(xout_rgb.input)
        
        xout_depth = self.pipeline.createXLinkOut()
        xout_depth.setStreamName("depth")
        self.depth_camera.depth.link(xout_depth.input)
        
        self.device = dai.Device(self.pipeline)
        self.q_rgb = self.device.getOutputQueue(name="rgb", maxSize=4, blocking=False)
        self.q_depth = self.device.getOutputQueue(name="depth", maxSize=4, blocking=False)
        
        self.timer = self.create_timer(0.1, self.publish_data)

    def publish_data(self):
        in_rgb = self.q_rgb.tryGet()
        in_depth = self.q_depth.tryGet()
        
        if in_rgb is not None and in_depth is not None:
            frame_rgb = in_rgb.getCvFrame()
            frame_depth = in_depth.getCvFrame()

            # Convert to ROS2 Image messages
            rgb_msg = self.bridge.cv2_to_imgmsg(frame_rgb, encoding="bgr8")
            depth_msg = self.bridge.cv2_to_imgmsg(frame_depth, encoding="mono16")
            
            self.publisher_rgb.publish(rgb_msg)
            self.publisher_depth.publish(depth_msg)
            self.get_logger().info('Publishing RGB and Depth images')

def main(args=None):
    rclpy.init(args=args)
    node = OAKDCameraNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
