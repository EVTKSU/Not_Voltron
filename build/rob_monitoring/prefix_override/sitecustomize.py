import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/sam/ros2_ws/src/rob_monitoring/install/rob_monitoring'
