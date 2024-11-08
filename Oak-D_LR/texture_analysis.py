#!/usr/bin/env python3

import cv2
import depthai as dai
import numpy as np
from skimage.feature import local_binary_pattern

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

# Gabor filter parameters
ksize = 31  # Size of the filter returned.
sigma = 4.0  # Standard deviation of the gaussian envelope.
theta = np.pi / 4  # Orientation of the normal to the parallel stripes.
lambd = 10.0  # Wavelength of the sinusoidal factor.
gamma = 0.5  # Spatial aspect ratio.
psi = 0  # Phase offset.

# LBP parameters
radius = 3
n_points = 8 * radius

# Connect to device and start pipeline
with dai.Device(pipeline) as device:
    qRgb = device.getOutputQueue(name="rgb", maxSize=4, blocking=False)

    # Create Gabor kernel
    g_kernel = cv2.getGaborKernel((ksize, ksize), sigma, theta, lambd, gamma, psi, ktype=cv2.CV_32F)

    while True:
        inRgb = qRgb.get()
        frame = inRgb.getCvFrame()

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Apply Gabor filter
        filtered_img = cv2.filter2D(gray, cv2.CV_8UC3, g_kernel)

        # Apply Local Binary Patterns
        lbp = local_binary_pattern(gray, n_points, radius, method='uniform')
        lbp = np.uint8(255 * lbp / lbp.max())

        # Display the resulting frames
        cv2.imshow('Original Frame', frame)
        cv2.imshow('Gabor Filtered', filtered_img)
        cv2.imshow('Local Binary Pattern', lbp)

        if cv2.waitKey(1) == ord('q'):
            break
