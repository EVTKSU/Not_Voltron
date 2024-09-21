from setuptools import setup

package_name = 'oakd_camera_node'

setup(
    name=package_name,
    version='0.0.1',
    packages=[package_name],
    install_requires=['setuptools', 'depthai', 'opencv-python', 'cv_bridge'],
    entry_points={
        'console_scripts': [
            'oakd_camera_node = oakd_camera_node:main'
        ],
    },
)
