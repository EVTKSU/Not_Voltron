#!/usr/bin/env python3

import cv2
import depthai as dai
import numpy as np

def region_of_interest(img, vertices):
    mask = np.zeros_like(img)
    cv2.fillPoly(mask, vertices, 255)
    masked = cv2.bitwise_and(img, mask)
    return masked

def draw_lines(img, lines):
    if lines is not None:
        for line in lines:
            for x1,y1,x2,y2 in line:
                # Calculate angle in degrees
                angle = np.degrees(np.arctan2(y2 - y1, x2 - x1))
                # Filter lines by angle
                if 30 < abs(angle) < 150:
                    cv2.line(img, (x1,y1), (x2,y2), (0,255,0), 2)

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
        blur = cv2.GaussianBlur(gray, (5, 5), 0)

        edges = cv2.Canny(blur, 50, 150)

        # Define a polygonal ROI
        imshape = frame.shape
        vertices = np.array([[(0, imshape[0]), (imshape[1]//2 - 50, imshape[0]//2 + 50),
                              (imshape[1]//2 + 50, imshape[0]//2 + 50), (imshape[1], imshape[0])]],
                            dtype=np.int32)
        masked_edges = region_of_interest(edges, vertices)

        # Hough Transform parameters
        lines = cv2.HoughLinesP(masked_edges, rho=1, theta=np.pi/180, threshold=50,
                                minLineLength=100, maxLineGap=50)

        line_image = np.zeros_like(frame)
        draw_lines(line_image, lines)

        # Combine original image with line image
        combo = cv2.addWeighted(frame, 0.8, line_image, 1, 0)

        # Display the resulting frames
        cv2.imshow('Original Frame', frame)
        cv2.imshow('Lane Lines', combo)

        if cv2.waitKey(1) == ord('q'):
            break
