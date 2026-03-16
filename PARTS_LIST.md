# Maxwell - Parts List

## Required Parts

| # | Component | Qty | Est. Price | Search Keywords |
|---|-----------|-----|-----------|-----------------|
| 1 | ESP32-CAM (with OV2640 camera) | 1 | $6-10 | "ESP32-CAM MB" (get the one with USB board) |
| 2 | 2WD Robot Car Chassis Kit | 1 | $10-15 | "2WD robot car chassis Arduino" |
| 3 | L298N Motor Driver Module | 1 | $3-5 | "L298N dual H-bridge" |
| 4 | 18650 Batteries | 2 | $4-6 | "18650 rechargeable 3.7V" |
| 5 | 18650 Battery Holder (2-slot w/ switch) | 1 | $1-2 | "2x 18650 battery holder with switch" |
| 6 | Jumper Wires (F-F, M-F) | 1 pack | $2-3 | "dupont jumper wires kit" |
| 7 | TP4056 Charging Module (USB-C) | 1 | $1-2 | "TP4056 USB-C lithium charger module" |
| 8 | Pogo Pins (spring-loaded, 2-pin) | 1 pair | $1-2 | "pogo pin spring loaded connector 2P" |
| 9 | Copper Tape (adhesive) | 1 roll | $1-2 | "copper foil tape adhesive conductive" |
| 10 | 18650 USB Charger (backup/initial) | 1 | $5-8 | "18650 battery charger USB" |

**Estimated Total: $35-55**

## What comes in the 2WD Chassis Kit

- 2x DC gear motors (3-6V)
- 2x Wheels
- 1x Caster wheel (front)
- 1x Acrylic chassis plate (sometimes 2 plates)
- Mounting screws and standoffs
- Battery box (you can use this OR the 18650 holder above)

## Build Phases

| Phase | What | Folder |
|-------|------|--------|
| 1 | Motor test — verify wiring | phase1_motor_control/ |
| 2 | Camera test — verify WiFi stream | phase2_camera/ |
| 3 | Full FPV car — camera + PS5 controller | phase3_full_car/ |
| 4 | Charging dock — drive in and charge | phase4_charging_dock/ |

## Wiring Diagram — Car

```
ESP32-CAM + L298N + TP4056 Wiring
==================================

Power (normal driving):
  18650 Batteries (+) --> TP4056 BAT+ --> L298N 12V input
  18650 Batteries (-) --> TP4056 BAT- --> L298N GND
  L298N 5V output     --> ESP32-CAM 5V
  L298N GND           --> ESP32-CAM GND

Charging (TP4056 on the car):
  Dock Pogo Pin (+)   --> TP4056 IN+ (or USB-C port)
  Dock Pogo Pin (-)   --> TP4056 IN-
  Copper pads on rear of car connect to pogo pins

Motor A (Left):
  ESP32-CAM GPIO 14   --> L298N IN1
  ESP32-CAM GPIO 15   --> L298N IN2
  L298N OUT1          --> Left Motor (+)
  L298N OUT2          --> Left Motor (-)

Motor B (Right):
  ESP32-CAM GPIO 13   --> L298N IN3
  ESP32-CAM GPIO  2   --> L298N IN4
  L298N OUT3          --> Right Motor (+)
  L298N OUT4          --> Right Motor (-)

Note: Remove the jumpers on ENA/ENB on the L298N
      to use full speed, or connect to PWM pins for
      speed control.
```

## Wiring Diagram — Charging Dock

```
Charging Dock (the "Supercharger")
===================================

  USB 5V Adapter --> Pogo Pin (+) --> contacts Car Copper Pad (+)
  USB GND        --> Pogo Pin (-) --> contacts Car Copper Pad (-)

  The TP4056 module ON THE CAR receives 5V from the dock
  and charges the 18650 batteries automatically.

  TP4056 LED indicators (built-in):
    Red  = Charging
    Blue = Fully charged

Physical layout (top view):

  ┌─────────────────────────┐
  │     CHARGING DOCK       │
  │                         │
  │   ┌──┐           ┌──┐  │
  │   │+ │  ← pogo → │- │  │ ← spring-loaded pins
  │   └──┘   pins    └──┘  │
  │         ═══════         │ ← guide rails (cardboard/3D print)
  │        /       \        │
  │       /   CAR   \       │ ← car drives in here
  │      /  drives  \       │
  └─────/   in here  \─────┘

  Car rear (where copper pads are):

  ┌─────────────────┐
  │     CAR REAR    │
  │  ┌────┐  ┌────┐ │
  │  │ +  │  │ -  │ │ ← copper tape pads
  │  └────┘  └────┘ │
  └─────────────────┘
```

## ESP32-CAM Pinout Reference

```
                 ┌─────────────┐
                 │   ESP32-CAM  │
                 │  ┌───────┐  │
            3V3 ─┤  │CAMERA │  ├─ 5V
           IO16 ─┤  │       │  ├─ GND
           IO0  ─┤  │       │  ├─ IO12
           GND  ─┤  └───────┘  ├─ IO13  --> IN3
           VCC  ─┤    [LED]    ├─ IO15  --> IN2
           IO14 ─┤             ├─ IO14  --> IN1
           IO2  ─┤             ├─ IO2   --> IN4
           IO4  ─┤  [SD CARD]  ├─ IO4   (flash LED)
                 └─────────────┘
```

## Optional Upgrades (later)

| Component | Price | What it adds |
|-----------|-------|-------------|
| HC-SR04 Ultrasonic Sensor | $2-3 | Obstacle avoidance / auto-park into dock |
| LEDs (white + red) | $1 | Headlights & taillights |
| Buzzer | $0.50 | Horn! |
| Servo SG90 | $2-3 | Pan camera left/right |
| 3D printed Tesla shell | $10-20 | Looking sick |
