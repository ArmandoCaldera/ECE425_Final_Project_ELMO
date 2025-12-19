import cv2
import numpy as np
import time
import pygame

# --------------------------------------------------
# Paths to YOLOv3-tiny model files (same folder)
# --------------------------------------------------
YOLO_CFG = "yolov3-tiny.cfg"
YOLO_WEIGHTS = "yolov3-tiny.weights"
COCO_NAMES = "coco.names"

# --------------------------------------------------
# Load class names from COCO
# --------------------------------------------------
with open(COCO_NAMES, "r") as f:
    CLASS_NAMES = [line.strip() for line in f.readlines()]

# --------------------------------------------------
# Objects we care about (must match COCO names)
# --------------------------------------------------
TARGET_CLASSES = {
    "backpack",
    "handbag",
    "book",
    "cell phone",
    "laptop",
    "keyboard",
    "mouse",
    "tv",
    "bottle",
    "cup",
    "chair",
    "remote",
    "clock",
    "refrigerator",
    "tvmonitor"   # some variants use this name
}

# --------------------------------------------------
# Map each target class to a specific audio file
# --------------------------------------------------
AUDIO_FILES = {
    "backpack":    "backpack_pcm.wav",
    "handbag":     "handbag_pcm.wav",
    "book":        "book_pcm.wav",
    "cell phone":  "cell_phone_pcm.wav",
    "laptop":      "Laptop_pcm.wav",
    "keyboard":    "keyboard_pcm.wav",
    "mouse":       "mouse_pcm.wav",
    "tv":          "tv_pcm.wav",
    "bottle":      "bottle_pcm.wav",
    "cup":         "cup_pcm.wav",
    "chair":       "chair_pcm.wav",
    "remote":      "remote_pcm.wav",
    "clock":       "clock_pcm.wav",
    "refrigerator":"refrigerator_pcm.wav",
    "tvmonitor":   "tvmonitor_pcm.wav"   # reuse TV audio if needed
}

# How long the object must be continuously visible
# (reduced from 2.0s -> 1.0s)
REQUIRED_VISIBILITY_SECONDS = 1.0

print("Loading YOLOv3-tiny model...")
net = cv2.dnn.readNetFromDarknet(YOLO_CFG, YOLO_WEIGHTS)

# --------------------------------------------------
# Get YOLO output layer names (version-safe)
# --------------------------------------------------
layer_names = net.getLayerNames()
output_layer_indices = net.getUnconnectedOutLayers()

try:
    output_layer_indices = output_layer_indices.flatten()
except Exception:
    pass

output_layer_names = [layer_names[int(i) - 1] for i in output_layer_indices]

print("Model loaded. Number of classes:", len(CLASS_NAMES))

# --------------------------------------------------
# Initialize pygame mixer for audio
# --------------------------------------------------
pygame.mixer.init()

# Track when we first saw each object class and
# whether we've already played its audio recently
class_visibility_start = {}   # class_name -> timestamp
class_last_play_time = {}     # class_name -> timestamp

# Cooldown AFTER we've played that object's sound once
# (increased from 5.0 -> 10.0 seconds)
MIN_TIME_BETWEEN_PLAYS = 10.0


def play_audio_for_class(label):
    """Play the audio file associated with a given label."""
    if label not in AUDIO_FILES:
        return

    filename = AUDIO_FILES[label]
    try:
        pygame.mixer.music.load(filename)
        pygame.mixer.music.play()
    except Exception as e:
        print(f"Error playing audio for {label}: {e}")


def main():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Error: could not open webcam.")
        return

    print("Press 'q' in the video window to quit.")

    global class_visibility_start, class_last_play_time

    # Speed-up controls
    frame_count = 0
    # Smoother video: run YOLO every 4th frame instead of every 3rd
    DETECT_EVERY_N_FRAMES = 4

    # Last detection results (for skipped frames)
    last_boxes = []
    last_confidences = []
    last_class_ids = []

    # For rate-limited debug printing
    last_debug_print_time = 0.0

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to read frame.")
            break

        height, width = frame.shape[:2]

        # --------------------------------------------------
        # BLOCK NEW DETECTIONS WHILE AUDIO IS PLAYING
        # --------------------------------------------------
        # While a voiceline is playing, we still show live video,
        # but we do NOT run YOLO / update detections.
        if pygame.mixer.music.get_busy():
            # Optionally you could overlay text indicating "Playing audio..."
            display_frame = cv2.resize(frame, (640, 360))
            cv2.imshow("YOLOv3-tiny Object Detection with Audio", display_frame)

            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                break

            # Skip the rest of the loop (no detection/visibility/audio updates)
            continue

        # --------------------------------------------------
        # Normal detection path (no audio currently playing)
        # --------------------------------------------------
        frame_count += 1
        run_detection = (frame_count % DETECT_EVERY_N_FRAMES) == 0

        if run_detection:
            # ------------------------------------------
            # Run YOLO only on this frame
            # ------------------------------------------
            blob = cv2.dnn.blobFromImage(
                frame,
                scalefactor=1 / 255.0,
                size=(320, 320),  # smaller = faster
                swapRB=True,
                crop=False
            )
            net.setInput(blob)
            layer_outputs = net.forward(output_layer_names)

            boxes = []
            confidences = []
            class_ids = []

            # Parse YOLO outputs
            for output in layer_outputs:
                for detection in output:
                    scores = detection[5:]
                    class_id = int(np.argmax(scores))
                    confidence = float(scores[class_id])

                    if confidence > 0.5:
                        center_x = int(detection[0] * width)
                        center_y = int(detection[1] * height)
                        w = int(detection[2] * width)
                        h = int(detection[3] * height)

                        x = int(center_x - w / 2)
                        y = int(center_y - h / 2)

                        boxes.append([x, y, w, h])
                        confidences.append(confidence)
                        class_ids.append(class_id)

            # Save for reuse on skipped frames
            last_boxes = boxes
            last_confidences = confidences
            last_class_ids = class_ids
        else:
            # ------------------------------------------
            # Reuse last detection results
            # ------------------------------------------
            boxes = last_boxes
            confidences = last_confidences
            class_ids = last_class_ids

        # Non-max suppression
        idxs = cv2.dnn.NMSBoxes(boxes, confidences, 0.5, 0.4)

        # Which target classes are visible this frame
        visible_classes_this_frame = set()
        detected_targets = []

        if len(idxs) > 0:
            for i in idxs.flatten():
                class_id = class_ids[i]
                label = CLASS_NAMES[class_id]

                if label not in TARGET_CLASSES:
                    continue

                x, y, w, h = boxes[i]
                conf = confidences[i]

                visible_classes_this_frame.add(label)
                detected_targets.append(label)

                # Draw bbox and label
                color = (0, 255, 0)
                cv2.rectangle(frame, (x, y), (x + w, y + h), color, 2)
                text = f"{label}: {conf:.2f}"
                text_y = y - 10 if y - 10 > 10 else y + 20
                cv2.putText(
                    frame,
                    text,
                    (x, text_y),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.5,
                    color,
                    2
                )

        # ----------------------------------------------
        # Handle "visible for >= 1 second" + audio
        # ----------------------------------------------
        now = time.time()

        # Update visibility timers
        for label in TARGET_CLASSES:
            if label in visible_classes_this_frame:
                if label not in class_visibility_start:
                    class_visibility_start[label] = now
            else:
                if label in class_visibility_start:
                    del class_visibility_start[label]

        # Decide which audios to play (respect cooldown)
        for label in visible_classes_this_frame:
            start_time = class_visibility_start.get(label, None)
            if start_time is None:
                class_visibility_start[label] = now
                continue

            visible_duration = now - start_time
            if visible_duration >= REQUIRED_VISIBILITY_SECONDS:
                last_play = class_last_play_time.get(label, 0)
                if (now - last_play) >= MIN_TIME_BETWEEN_PLAYS:
                    print(f"Playing audio for {label} (visible for {visible_duration:.2f}s)")
                    play_audio_for_class(label)
                    class_last_play_time[label] = now

        # ----------------------------------------------
        # Debug print (rate-limited so it doesn't tank FPS)
        # ----------------------------------------------
        if detected_targets:
            if now - last_debug_print_time >= 1.0:  # print at most once per second
                unique_targets = sorted(set(detected_targets))
                print("Detected:", ", ".join(unique_targets))
                last_debug_print_time = now

        # ----------------------------------------------
        # Show video (downscaled for smoother display)
        # ----------------------------------------------
        display_frame = cv2.resize(frame, (640, 360))
        cv2.imshow("YOLOv3-tiny Object Detection with Audio", display_frame)

        # Quit on 'q'
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
    pygame.mixer.quit()


if __name__ == "__main__":
    main()
