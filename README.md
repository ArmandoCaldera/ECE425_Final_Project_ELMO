# Object Detecting Autonomous RC Car (ECE 425 Final Project)

##Introduction

An autonomous rover built as **two parallel subsystems**:

- **Raspberry Pi 4**: real-time object detection (OpenCV DNN) + plays an associated audio clip (ELMO voiceline) when supported objects are detected.
- **TM4C123 (Tiva C LaunchPad)**: motor control + ultrasonic obstacle avoidance + OLED status display.

> The subsystems run independently (UART was planned but not completed). 

---

## Demo Summary (What it does)

### Raspberry Pi subsystem
- Captures frames from a USB webcam.
- Runs **YOLOv3-tiny** (final choice) for object detection.
- If a supported class is detected (ex: *bottle*, *laptop*, *backpack*), the Pi plays a matching `.wav` file.
- Audio is rate-limited to avoid spam:
  - object must be visible ≥ **1 second**
  - same object audio has a ≥ **10 second** cooldown

### TM4C subsystem
- Drives two DC motors using TB6612FNG (PWM + direction pins).
- Uses HC-SR04 ultrasonic sensor to detect obstacles.
- Displays rover state on SSD1315 OLED (I2C).
- Behavior loop:
  - go forward if distance is clear
  - stop and show WAITING when blocked
  - if still blocked, perform turn-left pulses until clear

### Block Diagram:
![Block Diagram](images/ECE425_block_diagram_final_project.png)

### State Diagram:
![State Diagram](images/ECE425_state_diagram_final_project.png)


---

## Hardware

### Components
| Component | Purpose |
|---|---|
| TM4C123GH6PM LaunchPad | Motor control, ultrasonic sensing, OLED display |
| Raspberry Pi 4 | Object detection + audio playback |
| Logitech USB Webcam | Live video capture |
| TB6612FNG | Dual H-bridge motor driver |
| HC-SR04 | Forward obstacle detection |
| SSD1315 OLED | I2C-based status display |
| 7.4V Li-ion Battery + regulators | System power |

---

## Wiring / Pin Mapping (TM4C)

### Motors (TB6612FNG)
| Motor | Signal | TM4C Pin | Notes |
|---|---|---|---|
| Right | PWMA | PB4 | PWM speed control |
| Right | AI1 | PC6 | direction |
| Right | AI2 | PC7 | direction |
| Left | PWMB | PB5 | PWM speed |
| Left | BI1 | PE2 | direction |
| Left | BI2 | PE3 | direction |

### Ultrasonic (HC-SR04)
| Signal | TM4C Pin |
|---|---|
| Trigger | PD2 |
| Echo | PE5 |

### OLED (SSD1315, I2C1)
| Signal | TM4C Pin |
|---|---|
| SCL | PA6 |
| SDA | PA7 |
| VCC | 3.3V |
| GND | GND |

---

## Video Demonstration:
Clip 1 [00:19]: https://youtube.com/shorts/MlB6wst_93M  \
Clip 2 [00:41]: https://youtu.be/d14usDj3cW0

---
