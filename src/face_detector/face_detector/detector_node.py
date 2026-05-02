import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy
from std_msgs.msg import Int32
from ultralytics import YOLO
import cv2

class FaceDetectorNode(Node):
    def __init__(self):
        super().__init__('face_detector')
        self.get_logger().info('Node started successfully!')
        qos = QoSProfile(depth=1, reliability=ReliabilityPolicy.BEST_EFFORT)
        self.publisher = self.create_publisher(Int32, 'face_position', qos)
        self.model = YOLO('PATH/TO/YOUR/model.pt')
        self.cap = cv2.VideoCapture(0)
        self.timer = self.create_timer(0.05, self.detect)

    def detect(self):
        ret, frame = self.cap.read()
        if not ret:
            return

        width = frame.shape[1]
        results = self.model(frame, verbose=False, imgsz=320)

        msg = Int32()

        if len(results[0].boxes) == 0:
            msg.data = 0
            label = "none"
        else:
            box = results[0].boxes[0]
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            x_center = (x1 + x2) / 2
            ratio = x_center / width

            if ratio < 0.4:
                msg.data = 1
                label = "LEFT"
            elif ratio > 0.6:
                msg.data = -1
                label = "RIGHT"
            else:
                msg.data = 0
                label = "CENTER"

            # draw bounding box and label
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(frame, label, (x1, y1 - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        # draw center line
        cv2.line(frame, (width//2, 0), (width//2, frame.shape[0]), (255, 0, 0), 1)

        cv2.namedWindow('Face Tracker', cv2.WINDOW_NORMAL)
        cv2.imshow('Face Tracker', frame)
        cv2.resizeWindow('Face Tracker', 800, 600)
        cv2.waitKey(1)

        self.publisher.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = FaceDetectorNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()