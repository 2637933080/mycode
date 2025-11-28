import numpy as np
import cv2

# Initialize face classifier (use correct path)
face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
)

# Read test image
img = cv2.imread('C:/Users/hp/Desktop/faces.jpg', cv2.IMREAD_COLOR)

# Convert to grayscale
gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

# Detect faces (fixed minNeighbors)
faces = face_cascade.detectMultiScale(gray, scaleFactor=1.2, minNeighbors=5)

# Draw rectangles on faces
for (x,y,w,h) in faces:
    cv2.rectangle(img, (x,y), (x+w,y+h), (255,255,0), 2)

# Show result
cv2.namedWindow('faces', cv2.WINDOW_AUTOSIZE)
cv2.imshow('faces', img)

# Wait for key press
cv2.waitKey(0)

# Close windows
cv2.destroyAllWindows()