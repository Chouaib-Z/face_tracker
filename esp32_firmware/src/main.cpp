#include <Arduino.h>
#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>
#include <ESP32Servo.h>

#define SERVO_PIN 13
#define WIFI_SSID "YOUR WIFI NAME"
#define WIFI_PASS "YOUR WIFI PASSWORD"
#define AGENT_IP  "IP ADDRESS OF YOUR PC"
#define AGENT_PORT 8888

Servo servo;
int current_angle = 90;

rcl_node_t node;
rcl_subscription_t subscriber;
std_msgs__msg__Int32 msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;

void subscription_callback(const void * msgin) {
    const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
    Serial.println("Received: " + String(msg->data));

    if (msg->data == 1) {
        current_angle = min(current_angle + 15, 180);
    } else if (msg->data == -1) {
        current_angle = max(current_angle - 15, 0);
    }

    servo.write(current_angle);
}

void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.println("Setup started");
    servo.attach(SERVO_PIN);
    servo.write(current_angle);

    set_microros_wifi_transports(WIFI_SSID, WIFI_PASS, AGENT_IP, AGENT_PORT);
    delay(2000);

    Serial.println("Transport set");

    allocator = rcl_get_default_allocator();
    rclc_support_init(&support, 0, NULL, &allocator);
    rclc_node_init_default(&node, "servo_node", "", &support);

    rclc_subscription_init_best_effort(
        &subscriber, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "face_position"
    );

    rclc_executor_init(&executor, &support.context, 1, &allocator);
    rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA);
}

void loop() {
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
    delay(100);
}