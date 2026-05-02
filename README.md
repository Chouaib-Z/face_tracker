# Face Tracker with YOLO, ROS2, and micro-ROS

A face tracking robot that uses YOLO for face detection on a PC and micro-ROS on an ESP32-S3 to control a servo motor.

## Architecture
- **PC:** Runs YOLO face detection and publishes face position to a ROS2 topic
- **ESP32-S3:** Subscribes to the topic via micro-ROS over WiFi and moves a servo accordingly

## Requirements
- ROS2 Jazzy
- Python 3
- Ultralytics YOLOv8
- PlatformIO
- ESP32-S3 board

## How to run

### PC side
```bash
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
ros2 run face_detector detector_node
```

### ESP32 side
- Update `WIFI_SSID`, `WIFI_PASS`, and `AGENT_IP` in `esp32_firmware/src/main.cpp`
- Flash with PlatformIO
