#!/usr/bin/env python3

import cv2
import depthai as dai
import numpy as np

# Kalman Filter setup
kalman = cv2.KalmanFilter(4, 2)
kalman.measurementMatrix = np.array([[1, 0, 0, 0],
                                     [0, 1, 0, 0]], dtype=np.float32)
kalman.transitionMatrix = np.array([[1, 0, 1, 0],
                                    [0, 1, 0, 1],
                                    [0, 0, 1, 0],
                                    [0, 0, 0, 1]], dtype=np.float32)
kalman.processNoiseCov = np.eye(4, dtype=np.float32) * 0.03

def auto_canny(image, sigma=0.33):
    # Compute the median of the pixel intensities
    v = np.median(image)
    # Apply automatic Canny edge detection using the computed median
    lower = int(max(0, (1.0 - sigma) * v))
    upper = int(min(255, (1.0 + sigma) * v))
    edged = cv2.Canny(image, lower, upper)
    return edged

# Create pipeline
pipeline = dai.Pipeline()

# Define sources and outputs
camRgb = pipeline.create(dai.node.ColorCamera)
xoutRgb = pipeline.create(dai.node.XLinkOut)

xoutRgb.setStreamName('rgb')

# Properties
camRgb.setPreviewSize(640, 480)
camRgb.setInterleaved(False)
camRgb.setColorOrder(dai.ColorCameraProperties.ColorOrder.BGR)

# Linking
camRgb.preview.link(xoutRgb.input)

# Parameters for temporal smoothing
frame_buffer = []
buffer_size = 5  # Number of frames to average

# Define ROI (e.g., bottom half of the image)
def get_roi(image):
    height, width = image.shape[:2]
    # Polygon coordinates for ROI
    roi_vertices = np.array([[
        (0, height),
        (0, int(height * 0.6)),
        (width, int(height * 0.6)),
        (width, height)
    ]], dtype=np.int32)
    mask = np.zeros_like(image)
    cv2.fillPoly(mask, roi_vertices, 255)
    masked_image = cv2.bitwise_and(image, mask)
    return masked_image

# Connect to device and start pipeline
with dai.Device(pipeline) as device:
    qRgb = device.getOutputQueue(name="rgb", maxSize=buffer_size, blocking=False)

    while True:
        inRgb = qRgb.get()
        frame = inRgb.getCvFrame()
        original_frame = frame.copy()  # Keep a copy for final display

        # Convert to HSV color space
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        # Define range of green color in HSV
        lower_green = np.array([35, 40, 40])
        upper_green = np.array([85, 255, 255])

        # Threshold the HSV image to get only green colors (grass)
        grass_mask = cv2.inRange(hsv, lower_green, upper_green)

        # Apply ROI to grass mask
        grass_mask = get_roi(grass_mask)

        # Apply adaptive Canny edge detection on the grass mask
        edges = auto_canny(grass_mask)

        # Apply temporal smoothing
        frame_buffer.append(edges)
        if len(frame_buffer) > buffer_size:
            frame_buffer.pop(0)
        avg_edges = np.mean(frame_buffer, axis=0).astype(np.uint8)
        _, avg_edges = cv2.threshold(avg_edges, 50, 255, cv2.THRESH_BINARY)

        # Find contours of the edges
        contours, _ = cv2.findContours(avg_edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        # Assume the largest contour by area is the grass edge
        if contours:
            largest_contour = max(contours, key=cv2.contourArea)

            # Get the x, y coordinates of the contour's centroid
            moments = cv2.moments(largest_contour)
            if moments['m00'] != 0:
                cx = moments['m10'] / moments['m00']
                cy = moments['m01'] / moments['m00']
                measured = np.array([[np.float32(cx)], [np.float32(cy)]], dtype=np.float32)
                kalman.correct(measured)
            else:
                measured = np.array([[0], [0]], dtype=np.float32)
                kalman.correct(measured)

            prediction = kalman.predict()
            pred_x = int(prediction[0][0])
            pred_y = int(prediction[1][0])

            # Draw the contour and the predicted line
            cv2.drawContours(original_frame, [largest_contour], -1, (0, 255, 0), 2)
            # Overlay a thick red line over the detected edge
            cv2.line(original_frame, (pred_x - 50, pred_y), (pred_x + 50, pred_y), (0, 0, 255), 5)
        else:
            # If no contours found, still predict using Kalman filter
            prediction = kalman.predict()
            pred_x = int(prediction[0][0])
            pred_y = int(prediction[1][0])
            # Overlay a thick red line over the predicted edge
            cv2.line(original_frame, (pred_x - 50, pred_y), (pred_x + 50, pred_y), (0, 0, 255), 5)

        # Display the resulting frames
        cv2.imshow('Original Frame with Edge', original_frame)
        cv2.imshow('Grass Mask', grass_mask)
        cv2.imshow('Smoothed Edges', avg_edges)

        if cv2.waitKey(1) == ord('q'):
            break
