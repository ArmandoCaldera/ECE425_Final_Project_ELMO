import cv2
import numpy as np

# Path to your model files in the same folder as this script
PROTOTXT = "MobileNetSSD_deploy.prototxt"
MODEL = "MobileNetSSD_deploy.caffemodel"

# Load the network
print("Loading model...")
net = cv2.dnn.readNetFromCaffe(PROTOTXT, MODEL)

# Class labels for this MobileNet-SSD (standard 20 classes + background)
# Note: some variants may differ slightly. Adjust if needed.
CLASSES = [
    "background", "aeroplane", "bicycle", "bird", "boat",
    "bottle", "bus", "car", "cat", "chair",
    "cow", "diningtable", "dog", "horse", "motorbike",
    "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor"
]

# Which objects we care about
TARGET_CLASSES = {"bottle", "chair", "cup", "apple"}  # 'cup'/'apple' may or may not exist depending on model

# Some models don't have 'cup' or 'apple' in CLASSES.
# If they don't exist, they’ll just never be detected – that's okay for now.


def get_target_label(class_id):
    if class_id < len(CLASSES):
        label = CLASSES[class_id]
        if label in TARGET_CLASSES:
            return label
    return None


def main():
    # Open the webcam
    cap = cv2.VideoCapture(0)  # 0 = /dev/video0

    if not cap.isOpened():
        print("Error: Could not open webcam.")
        return

    print("Press 'q' to quit.")

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame")
            break

        # Prepare blob for MobileNet-SSD
        # MobileNet-SSD expects 300x300 BGR image
        (h, w) = frame.shape[:2]
        blob = cv2.dnn.blobFromImage(
            cv2.resize(frame, (300, 300)),
            0.007843,  # scale factor
            (300, 300),
            127.5      # mean subtraction
        )

        net.setInput(blob)
        detections = net.forward()

        detected_targets = []

        # Loop over detections
        for i in range(detections.shape[2]):
            confidence = detections[0, 0, i, 2]

            # Confidence threshold
            if confidence > 0.5:
                class_id = int(detections[0, 0, i, 1])
                label = get_target_label(class_id)
                if label is None:
                    continue

                # Compute bounding box
                box = detections[0, 0, i, 3:7] * np.array([w, h, w, h])
                (startX, startY, endX, endY) = box.astype("int")

                # Save for summary
                detected_targets.append(label)

                # Draw box and label
                text = f"{label}: {confidence:.2f}"
                cv2.rectangle(frame, (startX, startY), (endX, endY), (0, 255, 0), 2)
                y = startY - 15 if startY - 15 > 15 else startY + 15
                cv2.putText(frame, text, (startX, y),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        # Optionally, print what we saw in this frame
        if detected_targets:
            unique_targets = sorted(set(detected_targets))
            print("Detected:", ", ".join(unique_targets))

        # Show the video
        cv2.imshow("Object Detection", frame)

        # Quit on 'q'
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
