**Build the Docker Image:**
<br>
*Open a terminal in the directory containing your Dockerfile and run:*  
>docker build -t ros2_image .
<br>

**Run the Docker Container:**
<br>
>docker run -it ros2_image

*This setup creates a Docker image based on Ubuntu, installs any necessary dependencies, and runs your ROS 2 installer script inside the container.*
