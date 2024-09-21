import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/buba/evt/Not_Voltron/oak_d_ws/install/oakd_camera_node'
