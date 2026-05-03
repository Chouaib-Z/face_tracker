#include <Arduino.h>
#include <micro_ros_arduino.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h> 
#include <ESP32Servo.h>

// ==========================================
// CONFIGURATION WI-FI ET ROS 2
// ==========================================
#define WIFI_SSID "TON_NOM_DE_WIFI"
#define WIFI_MDP "TON_MOT_DE_PASSE"
#define IP_AGENT "192.168.1.100" // L'adresse IP locale du PC qui fait tourner l'agent ROS 2
#define PORT_AGENT 8888          // Port par défaut de l'agent micro-ROS

// ==========================================
// CONFIGURATION DU SERVOMOTEUR
// ==========================================
Servo monServo;
const int SERVO_PIN = 10; 

// ==========================================
// VARIABLES DU MODE RADAR
// ==========================================
unsigned long dernier_visage_temps = 0;
const unsigned long DELAI_RADAR_MS = 3000; // Déclenchement après 3 secondes sans visage
int position_servo = 90;                   // Centre par défaut
int vitesse_balayage = 1;                  // Vitesse du radar (1 degré par boucle)

// ==========================================
// VARIABLES MICRO-ROS
// ==========================================
rcl_subscription_t subscriber;
std_msgs__msg__Int32 msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

// Boucle d'erreur en cas de problème d'initialisation
void error_loop(){
  while(1){
    delay(100);
  }
}

// ==========================================
// CALLBACK : RÉCEPTION DE LA POSITION VISAGE
// ==========================================
void subscription_callback(const void * msgin) {
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
  
  // 1. REINITIALISATION DU RADAR (Un visage est détecté)
  dernier_visage_temps = millis();
  
  // 2. Récupération de l'angle calculé par l'ordinateur
  position_servo = msg->data; 
  
  // 3. Sécurité : Butées mécaniques pour protéger le servomoteur
  if (position_servo > 170) position_servo = 170;
  if (position_servo < 10) position_servo = 10;
  
  // 4. Mouvement vers la cible
  monServo.write(position_servo);
}

// ==========================================
// INITIALISATION DU SYSTÈME
// ==========================================
void setup() {
  Serial.begin(115200);

  // 1. Setup du servomoteur
  monServo.attach(SERVO_PIN);
  monServo.write(position_servo);
  dernier_visage_temps = millis(); // Lancement du chronomètre

  // 2. Connexion Wi-Fi à l'agent micro-ROS
  set_microros_wifi_transports(WIFI_SSID, WIFI_MDP, IP_AGENT, PORT_AGENT);
  delay(2000);

  allocator = rcl_get_default_allocator();

  // 3. Initialisation de l'infrastructure ROS 2
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
  RCCHECK(rclc_node_init_default(&node, "face_tracker_esp32", "", &support));

  // 4. Création du Subscriber (écoute sur le topic "cible_visage")
  RCCHECK(rclc_subscription_init_default(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "cible_visage"));

  // 5. Configuration de l'Exécuteur
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
}

// ==========================================
// BOUCLE PRINCIPALE
// ==========================================
void loop() {
  // 1. L'exécuteur traite les messages entrants de ROS 2
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));

  // 2. LOGIQUE DU MODE RADAR AUTONOME
  if (millis() - dernier_visage_temps > DELAI_RADAR_MS) {
    
    // Avance d'un pas
    position_servo += vitesse_balayage;
    
    // Inversion de la direction aux extrémités (20° et 160°)
    if (position_servo >= 160 || position_servo <= 20) {
      vitesse_balayage = -vitesse_balayage; 
    }
    
    // Application de la position de recherche
    monServo.write(position_servo);
    
    // Micro-délai pour définir la vitesse visuelle du radar
    delay(15); 
  }
}
