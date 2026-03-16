/*
 * Maxwell - Phase 1: Basic Motor Control
 *
 * This phase tests that your motors and wiring work correctly.
 * The car will: go forward, stop, go backward, stop, turn left,
 * turn right, then stop. It repeats every 10 seconds.
 *
 * Upload this first to verify your wiring before adding WiFi/camera.
 *
 * Wiring:
 *   ESP32 GPIO 14 -> L298N IN1 (Left Motor)
 *   ESP32 GPIO 15 -> L298N IN2 (Left Motor)
 *   ESP32 GPIO 13 -> L298N IN3 (Right Motor)
 *   ESP32 GPIO  2 -> L298N IN4 (Right Motor)
 */

// Motor A (Left) pins
#define IN1 14
#define IN2 15

// Motor B (Right) pins
#define IN3 13
#define IN4 2

void setup() {
  Serial.begin(115200);
  Serial.println("Maxwell - Phase 1: Motor Test");

  // Set motor pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Start with motors off
  stopMotors();

  delay(2000); // Wait 2 seconds before starting
  Serial.println("Starting motor test...");
}

void loop() {
  Serial.println(">> FORWARD");
  moveForward();
  delay(2000);

  Serial.println(">> STOP");
  stopMotors();
  delay(1000);

  Serial.println(">> BACKWARD");
  moveBackward();
  delay(2000);

  Serial.println(">> STOP");
  stopMotors();
  delay(1000);

  Serial.println(">> TURN LEFT");
  turnLeft();
  delay(1500);

  Serial.println(">> STOP");
  stopMotors();
  delay(1000);

  Serial.println(">> TURN RIGHT");
  turnRight();
  delay(1500);

  Serial.println(">> STOP");
  stopMotors();
  delay(3000);

  Serial.println("--- Cycle complete, restarting ---\n");
}

// Both motors forward
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Both motors backward
void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// Left motor backward, right motor forward (turns left)
void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Left motor forward, right motor backward (turns right)
void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// All motors off
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
