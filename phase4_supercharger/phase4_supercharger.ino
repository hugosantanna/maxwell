/*
 * Maxwell - Phase 4: Supercharger Dock (Auto-Plug Cable)
 *
 * This runs on an Arduino Nano inside a vertical Supercharger stall.
 * A flexible silicone charging cable hangs from the top of the stall,
 * held up by a servo. When the car parks nearby, the IR sensor detects
 * it, the servo releases the cable, and gravity drops it down so the
 * magnetic tip snaps onto the car's charge port — just like a real
 * Tesla Supercharger cable.
 *
 * The car side handles charging via the TP4056 module.
 * This code only controls the dock: detect, lower cable, indicate, retract.
 *
 * Physical build:
 *   - Vertical stall ~15-20cm tall (3D printed or cardboard)
 *   - Servo mounted at the TOP of the stall
 *   - Flexible silicone cable (26AWG) runs from 5V USB through a
 *     guide loop on the servo horn, down to a magnetic connector
 *   - Cable hangs naturally when released (gravity does the work)
 *   - Magnetic tip clicks onto copper/magnetic pads on the car
 *
 *   Side view:
 *
 *     ┌─────┐
 *     │STALL│
 *     │     │  [servo] ← holds cable up
 *     │     │   │
 *     │     │   │ cable (silicone wire in heat shrink)
 *     │     │   │
 *     │     │   ● magnetic connector tip
 *     │     │   ↕
 *     │     │  [CAR charge port]
 *     │ IR  │
 *     │sensor → detects car
 *     └─────┘
 *
 *   Servo positions:
 *     UP (retracted) = cable held against top of stall
 *     DOWN (released) = cable drops, magnetic tip reaches car
 *
 * Wiring (Arduino Nano):
 *   D2  <-- IR sensor OUT (FC-51, LOW when object detected)
 *   D9  --> SG90 servo signal (mounted at top of stall)
 *   D4  --> 220 ohm --> Red LED --> GND   (charging)
 *   D5  --> 220 ohm --> Green LED --> GND (complete)
 *   5V  --> IR sensor VCC, Servo VCC
 *   GND --> IR sensor GND, Servo GND
 *
 * Charging cable path:
 *   USB 5V  --> wire up stall --> through servo guide loop --> down to magnetic tip (+)
 *   USB GND --> wire up stall --> through servo guide loop --> down to magnetic tip (-)
 *
 * The cable is always connected to 5V — the servo just controls
 * whether the cable hangs down (reaches car) or is held up (retracted).
 * The TP4056 on the car handles safe charging once contact is made.
 */

#include <Servo.h>

// Pins
#define IR_SENSOR_PIN  2   // IR obstacle sensor (FC-51) at base of stall
#define SERVO_PIN      9   // SG90 servo at top of stall
#define LED_CHARGING   4   // Red LED on stall face
#define LED_COMPLETE   5   // Green LED on stall face

// Servo positions — adjust these to match your stall height
// The servo horn has a small loop/hook that holds the cable
#define CABLE_UP    160   // Cable pulled up tight against stall (retracted)
#define CABLE_DOWN   40   // Cable released, hangs down to car level

// Smooth cable lowering — moves the servo in small steps for a
// realistic slow-drop effect instead of snapping instantly
#define LOWER_STEP_DEG    2    // Degrees per step
#define LOWER_STEP_MS     30   // Milliseconds between steps
#define RETRACT_STEP_DEG  3    // Slightly faster retraction
#define RETRACT_STEP_MS   20

// Timing
#define DETECT_DELAY_MS     2000      // Wait for car to settle before lowering
#define CHARGE_CHECK_MS     2000      // How often to check if car is still there
#define CHARGE_TIMEOUT_MS   10800000  // 3 hours max (safety cutoff)

Servo cableServo;
int currentServoPos;

enum DockState {
  WAITING,       // No car, cable held up
  CAR_DETECTED,  // Car sensed, waiting before lowering
  LOWERING,      // Servo slowly releasing cable
  CHARGING,      // Cable down, magnetic tip connected, charging
  COMPLETE,      // Charge done (timeout)
  RETRACTING     // Servo pulling cable back up
};

DockState state = WAITING;
unsigned long stateTimer = 0;
unsigned long chargeStartTime = 0;

// Move servo smoothly from current position toward target
// Returns true when target is reached
bool moveServoStep(int target, int stepDeg, int stepMs) {
  if (currentServoPos == target) return true;

  if (currentServoPos < target) {
    currentServoPos = min(currentServoPos + stepDeg, target);
  } else {
    currentServoPos = max(currentServoPos - stepDeg, target);
  }
  cableServo.write(currentServoPos);
  delay(stepMs);
  return (currentServoPos == target);
}

void setup() {
  Serial.begin(9600);
  Serial.println("=== Maxwell Supercharger ===");
  Serial.println("Cable retracted. Waiting for car...\n");

  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(LED_CHARGING, OUTPUT);
  pinMode(LED_COMPLETE, OUTPUT);

  // Start with cable held up
  cableServo.attach(SERVO_PIN);
  currentServoPos = CABLE_UP;
  cableServo.write(CABLE_UP);

  digitalWrite(LED_CHARGING, LOW);
  digitalWrite(LED_COMPLETE, LOW);

  delay(1000);
}

bool carDetected() {
  // FC-51 IR sensor at base of stall: LOW = car is there
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
        Serial.println("Car detected. Waiting for it to settle...");
        stateTimer = now;
        state = CAR_DETECTED;
      }
      break;

    case CAR_DETECTED:
      // Blink red while waiting
      setLEDs((now / 250) % 2, false);

      if (!carDetected()) {
        Serial.println("Car moved away.");
        setLEDs(false, false);
        state = WAITING;
      } else if (now - stateTimer >= DETECT_DELAY_MS) {
        Serial.println("Lowering cable...");
        state = LOWERING;
      }
      break;

    case LOWERING:
      // Slowly lower cable toward car
      setLEDs(true, false);
      {
        bool reached = moveServoStep(CABLE_DOWN, LOWER_STEP_DEG, LOWER_STEP_MS);
        if (reached) {
          Serial.println("Cable connected. Charging!");
          chargeStartTime = now;
          stateTimer = now;
          state = CHARGING;
        }
      }
      break;

    case CHARGING:
      // Solid red while charging
      setLEDs(true, false);

      if (now - stateTimer >= CHARGE_CHECK_MS) {
        stateTimer = now;
        if (!carDetected()) {
          Serial.println("Car disconnected. Retracting cable...");
          state = RETRACTING;
        }
      }

      // Safety timeout
      if (now - chargeStartTime >= CHARGE_TIMEOUT_MS) {
        Serial.println("Charge timeout. Retracting cable...");
        state = COMPLETE;
      }
      break;

    case COMPLETE:
      // Green = done
      setLEDs(false, true);
      Serial.println("Charge complete!");

      if (!carDetected()) {
        delay(500);
        state = RETRACTING;
      }
      break;

    case RETRACTING:
      // Pull cable back up
      {
        bool reached = moveServoStep(CABLE_UP, RETRACT_STEP_DEG, RETRACT_STEP_MS);
        if (reached) {
          setLEDs(false, false);
          Serial.println("Cable retracted. Ready.\n");
          state = WAITING;
        }
      }
      break;
  }

  delay(50);
}
