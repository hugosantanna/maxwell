/*
 * Maxwell - Phase 4: Supercharger Dock (Auto-Plug)
 *
 * This runs on an Arduino Nano inside the Supercharger dock.
 * It detects when the car parks nearby using an IR sensor,
 * then swings a servo arm to plug a magnetic charging cable
 * into the car's charge port — just like a Tesla Supercharger.
 *
 * The car side handles charging via the TP4056 module.
 * This code only controls the dock: detect, plug, indicate, retract.
 *
 * Wiring (Arduino Nano):
 *   D2  <-- IR sensor OUT (FC-51, LOW when object detected)
 *   D9  --> SG90 servo signal
 *   D4  --> 220 ohm --> Red LED --> GND   (charging indicator)
 *   D5  --> 220 ohm --> Green LED --> GND (complete indicator)
 *   5V  --> IR sensor VCC, Servo VCC
 *   GND --> IR sensor GND, Servo GND
 *
 * Charging cable runs through the servo arm:
 *   5V USB  --> wire through arm --> magnetic connector (+)
 *   USB GND --> wire through arm --> magnetic connector (-)
 */

#include <Servo.h>

// Pins
#define IR_SENSOR_PIN  2   // IR obstacle sensor (FC-51)
#define SERVO_PIN      9   // SG90 servo
#define LED_CHARGING   4   // Red LED
#define LED_COMPLETE   5   // Green LED

// Servo positions (adjust these to match your physical build)
#define SERVO_RETRACTED 90  // Cable tucked away
#define SERVO_EXTENDED   0  // Cable swung out to car's charge port

// Timing
#define DETECT_DELAY_MS     1500  // Wait before plugging in (car settling)
#define CHARGE_CHECK_MS     2000  // How often to check if car is still there
#define CHARGE_TIMEOUT_MS 10800000  // 3 hours max charge time (safety)

Servo chargerArm;

enum DockState {
  WAITING,      // No car detected, arm retracted
  CAR_DETECTED, // Car detected, waiting before plugging in
  PLUGGING_IN,  // Servo swinging cable to car
  CHARGING,     // Cable connected, charging in progress
  COMPLETE,     // Charge complete (timeout or car removed)
  RETRACTING    // Servo pulling cable back
};

DockState state = WAITING;
unsigned long stateTimer = 0;
unsigned long chargeStartTime = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("=== Maxwell Supercharger Dock ===");
  Serial.println("Waiting for car...\n");

  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(LED_CHARGING, OUTPUT);
  pinMode(LED_COMPLETE, OUTPUT);

  chargerArm.attach(SERVO_PIN);
  chargerArm.write(SERVO_RETRACTED);

  digitalWrite(LED_CHARGING, LOW);
  digitalWrite(LED_COMPLETE, LOW);

  delay(1000);
}

bool carDetected() {
  // FC-51 IR sensor: LOW = object detected
  return digitalRead(IR_SENSOR_PIN) == LOW;
}

void setLEDs(bool red, bool green) {
  digitalWrite(LED_CHARGING, red ? HIGH : LOW);
  digitalWrite(LED_COMPLETE, green ? HIGH : LOW);
}

void loop() {
  unsigned long now = millis();

  switch (state) {

    case WAITING:
      setLEDs(false, false);
      if (carDetected()) {
        Serial.println("Car detected! Waiting for it to settle...");
        stateTimer = now;
        state = CAR_DETECTED;
      }
      break;

    case CAR_DETECTED:
      // Blink red LED while waiting
      setLEDs((now / 250) % 2, false);
      if (!carDetected()) {
        Serial.println("Car moved away. Back to waiting.");
        setLEDs(false, false);
        state = WAITING;
      } else if (now - stateTimer >= DETECT_DELAY_MS) {
        Serial.println("Plugging in...");
        state = PLUGGING_IN;
      }
      break;

    case PLUGGING_IN:
      // Swing the servo arm to connect the cable
      chargerArm.write(SERVO_EXTENDED);
      setLEDs(true, false);
      Serial.println("Cable connected. Charging started!");
      chargeStartTime = now;
      stateTimer = now;
      state = CHARGING;
      break;

    case CHARGING:
      // Solid red LED while charging
      setLEDs(true, false);

      // Check periodically if car is still there
      if (now - stateTimer >= CHARGE_CHECK_MS) {
        stateTimer = now;

        if (!carDetected()) {
          Serial.println("Car removed. Retracting cable.");
          state = RETRACTING;
        }
      }

      // Safety timeout
      if (now - chargeStartTime >= CHARGE_TIMEOUT_MS) {
        Serial.println("Charge timeout reached. Retracting cable.");
        state = COMPLETE;
      }
      break;

    case COMPLETE:
      // Green LED = done
      setLEDs(false, true);
      Serial.println("Charge complete!");

      // Wait for car to leave, then retract
      if (!carDetected()) {
        delay(500);
        state = RETRACTING;
      }
      break;

    case RETRACTING:
      Serial.println("Retracting cable...");
      chargerArm.write(SERVO_RETRACTED);
      setLEDs(false, false);
      delay(1000);  // Wait for servo to finish moving
      Serial.println("Ready for next car.\n");
      state = WAITING;
      break;
  }

  delay(50);
}
