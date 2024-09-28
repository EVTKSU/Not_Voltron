#!/usr/bin/env python3

import cv2
import depthai as dai
import numpy as np

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
    # Output queue will be used to get the rgb frames from the output defined above
    qRgb = device.getOutputQueue(name="rgb", maxSize=4, blocking=False)

    while True:
        inRgb = qRgb.get()  # Get RGB frame
        frame = inRgb.getCvFrame()

        # Convert to HSV color space
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        # Define range of green color in HSV
        lower_green = np.array([35, 40, 40])
        upper_green = np.array([85, 255, 255])

        # Threshold the HSV image to get only green colors
        mask = cv2.inRange(hsv, lower_green, upper_green)

        # Bitwise-AND mask and original image
        res = cv2.bitwise_and(frame, frame, mask=mask)

        # Find edges of the masked area
        edges = cv2.Canny(mask, 50, 150)

        # Display the resulting frames
        cv2.imshow('Original Frame', frame)
        cv2.imshow('Green Mask', mask)
        cv2.imshow('Masked Result', res)
        cv2.imshow('Edges of Mask', edges)

        if cv2.waitKey(1) == ord('q'):
            break
