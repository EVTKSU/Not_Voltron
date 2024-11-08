#!/usr/bin/env python3

#canny filter edge detector

import cv2
import depthai as dai
import numpy as np

# Create pipeline
pipeline = dai.Pipeline()

# Define sources and outputs
camRgb = pipeline.create(dai.node.ColorCamera)
monoLeft = pipeline.create(dai.node.MonoCamera)
monoRight = pipeline.create(dai.node.MonoCamera)

xoutLeft = pipeline.create(dai.node.XLinkOut)
xoutRight = pipeline.create(dai.node.XLinkOut)
xoutRgb = pipeline.create(dai.node.XLinkOut)

xoutLeft.setStreamName('left')
xoutRight.setStreamName('right')
xoutRgb.setStreamName('rgb')

# Properties
camRgb.setBoardSocket(dai.CameraBoardSocket.CAM_A)
camRgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
camRgb.setInterleaved(False)
camRgb.setColorOrder(dai.ColorCameraProperties.ColorOrder.BGR)

monoLeft.setResolution(dai.MonoCameraProperties.SensorResolution.THE_400_P)
monoLeft.setBoardSocket(dai.CameraBoardSocket.LEFT)
monoRight.setResolution(dai.MonoCameraProperties.SensorResolution.THE_400_P)
monoRight.setBoardSocket(dai.CameraBoardSocket.RIGHT)

# Linking
monoLeft.out.link(xoutLeft.input)
monoRight.out.link(xoutRight.input)
camRgb.video.link(xoutRgb.input)

# Connect to device and start pipeline
with dai.Device(pipeline) as device:

    # Output queues
    leftQueue = device.getOutputQueue('left', maxSize=4, blocking=False)
    rightQueue = device.getOutputQueue('right', maxSize=4, blocking=False)
    rgbQueue = device.getOutputQueue('rgb', maxSize=4, blocking=False)

    threshold1 = 100
    threshold2 = 200

    print("Press 'q' to quit.")
    print("Adjust thresholds with keys '1'-'4'.")

    while True:
        leftIn = leftQueue.get()
        rightIn = rightQueue.get()
        rgbIn = rgbQueue.get()

        leftFrame = leftIn.getCvFrame()
        rightFrame = rightIn.getCvFrame()
        rgbFrame = rgbIn.getCvFrame()

        # For RGB image, convert to grayscale
        rgbGray = cv2.cvtColor(rgbFrame, cv2.COLOR_BGR2GRAY)

        # Apply Canny edge detector
        leftEdges = cv2.Canny(leftFrame, threshold1, threshold2)
        rightEdges = cv2.Canny(rightFrame, threshold1, threshold2)
        rgbEdges = cv2.Canny(rgbGray, threshold1, threshold2)

        # Show the frames
        cv2.imshow('Left Camera - Canny Edge Detection', leftEdges)
        cv2.imshow('Right Camera - Canny Edge Detection', rightEdges)
        cv2.imshow('RGB Camera - Canny Edge Detection', rgbEdges)

        key = cv2.waitKey(1)
        if key == ord('q'):
            break
        elif key == ord('1'):
            threshold1 = max(0, threshold1 - 10)
            print(f"Threshold1 decreased to {threshold1}")
        elif key == ord('2'):
            threshold1 = min(255, threshold1 + 10)
            print(f"Threshold1 increased to {threshold1}")
        elif key == ord('3'):
            threshold2 = max(0, threshold2 - 10)
            print(f"Threshold2 decreased to {threshold2}")
        elif key == ord('4'):
            threshold2 = min(255, threshold2 + 10)
            print(f"Threshold2 increased to {threshold2}")
