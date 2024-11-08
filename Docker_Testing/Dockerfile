# Use the desired Ubuntu base image
FROM ubuntu:20.04

# Set environment variables if needed
ENV DEBIAN_FRONTEND=noninteractive

# Update and install any prerequisites your script might need
RUN apt-get update && apt-get install -y \
    curl \
    gnupg2 \
    lsb-release \
    && rm -rf /var/lib/apt/lists/*

# Copy your ROS 2 installer script into the container
COPY your_ros2_installer.sh /tmp/

# Make the script executable
RUN chmod +x /tmp/your_ros2_installer.sh

# Run the installer script
RUN /tmp/your_ros2_installer.sh

# Set the default command
CMD ["/bin/bash"]
