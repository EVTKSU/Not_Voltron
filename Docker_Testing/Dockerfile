# Use the official ROS 2 Jazzy Jalisco base image
FROM ros:jazzy-ros-base

# Update and install necessary packages
RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    git \
    && rm -rf /var/lib/apt/lists/*

# Initialize rosdep
RUN rosdep init && rosdep update

# Set the working directory
WORKDIR /root/ros2_ws

# Source ROS 2 setup script in every new shell
RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc

# Default command to run when starting the container
CMD ["/bin/bash"]
