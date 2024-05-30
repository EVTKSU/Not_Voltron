import time
import depthai as dai
import cv2
import numpy as np
from datetime import datetime
import threading

def detect_green_and_process(frame):
    hsv_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    lower_green = np.array([40, 40, 40])
    upper_green = np.array([80, 255, 255])
    mask = cv2.inRange(hsv_frame, lower_green, upper_green)
    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    
    if len(contours) == 0:
        process_data("no green detected")
    else:
        largest_contour = max(contours, key=cv2.contourArea)
        (x, y, w, h) = cv2.boundingRect(largest_contour)
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        center_x, center_y = x + w // 2, y + h // 2
        process_data(f"{center_x},{center_y}")

def process_data(message):
    global status, recent_data
    if message == "no green detected":
        status = "No green detected"
    else:
        x, y = map(int, message.split(','))
        status = determine_status(y)
        rotate_arrow(y)
    recent_data.append((datetime.now(), message, status))

def determine_status(distance_mm):
    return f"Nearest Green is {distance_mm} mm away from the camera!"

def rotate_arrow(y_coordinate):
    arrow = cv2.imread('arrow.png', cv2.IMREAD_UNCHANGED)
    arrow_resized = cv2.resize(arrow, (arrow.shape[1] // 32, arrow.shape[0] // 32), interpolation=cv2.INTER_AREA)
    if y_coordinate < 100:
        rotated_arrow = cv2.rotate(arrow_resized, cv2.ROTATE_90_COUNTERCLOCKWISE)
    #50mm of play    
    elif y_coordinate > 150:
        rotated_arrow = cv2.rotate(arrow_resized, cv2.ROTATE_90_CLOCKWISE)
    else:
        rotated_arrow = arrow_resized  # No rotation if y_coordinate is exactly 100
    cv2.imshow("Rotated Arrow", rotated_arrow)
    cv2.waitKey(1)

def display_status():
    global status
    while True:
        print(f"Current status: {status}")
        time.sleep(0.1)

def main():
    global status, recent_data
    status = ""
    recent_data = []

    pipeline = dai.Pipeline()
    cam_rgb = pipeline.create(dai.node.ColorCamera)
    xout_rgb = pipeline.create(dai.node.XLinkOut)

    cam_rgb.setBoardSocket(dai.CameraBoardSocket.CAM_A)
    cam_rgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_800_P)
    xout_rgb.setStreamName("rgb")
    cam_rgb.video.link(xout_rgb.input)

    with dai.Device(pipeline) as device:
        q_rgb = device.getOutputQueue(name="rgb", maxSize=4, blocking=False)
        threading.Thread(target=display_status, daemon=True).start()

        while True:
            in_rgb = q_rgb.get()
            frame_rgb = in_rgb.getCvFrame()
            detect_green_and_process(frame_rgb)
            cv2.imshow("RGB", frame_rgb)
            if cv2.waitKey(1) == ord('q'):
                break

    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
