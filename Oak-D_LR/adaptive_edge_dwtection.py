#!/usr/bin/env python3

import cv2
import depthai as dai
import numpy as np

def auto_canny(image, sigma=0.33):
    # Compute the median of the single channel pixel intensities
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

# Connect to device and start pipeline
with dai.Device(pipeline) as device:
    qRgb = device.getOutputQueue(name="rgb", maxSize=4, blocking=False)

    while True:
        inRgb = qRgb.get()
        frame = inRgb.getCvFrame()

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Define ROI (e.g., bottom half of the image)
        roi = gray[gray.shape[0]//2:, :]

        # Apply adaptive Canny edge detection
        edges = auto_canny(roi)

        # Create an empty image to hold the edges in the full frame size
        full_edges = np.zeros_like(gray)
        full_edges[gray.shape[0]//2:, :] = edges

        # Display the resulting frames
        cv2.imshow('Original Frame', frame)
        cv2.imshow('Edges', full_edges)

        if cv2.waitKey(1) == ord('q'):
            break
