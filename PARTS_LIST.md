# Maxwell - Parts List

## Car Parts

| # | Component | Qty | Est. Price | Search on Amazon |
|---|-----------|-----|-----------|-----------------|
| 1 | ESP32-CAM with OV2640 camera + USB programmer | 1 | $6-10 | "ESP32-CAM MB WiFi Bluetooth" |
| 2 | 2WD Robot Car Chassis Kit | 1 | $10-15 | "2WD robot car chassis Arduino" |
| 3 | L298N Motor Driver Module | 1 | $3-5 | "L298N dual H-bridge motor driver" |
| 4 | 18650 Rechargeable Batteries (3.7V) | 2 | $4-6 | "18650 rechargeable battery 3.7V" |
| 5 | 18650 Battery Holder (2-slot with switch) | 1 | $1-2 | "18650 battery holder 2 slot switch" |
| 6 | TP4056 USB-C Lithium Charger Module | 1 | $1-2 | "TP4056 USB-C lithium charger module" |
| 7 | 2-Pin Magnetic Pogo Connector (car side) | 1 | $1-2 | "magnetic pogo pin connector 2 pin" |
| 8 | Jumper Wires (F-F and M-F) | 1 pack | $2-3 | "dupont jumper wire kit" |

**Car subtotal: ~$28-45**

## Supercharger Dock Parts

| # | Component | Qty | Est. Price | Search on Amazon |
|---|-----------|-----|-----------|-----------------|
| 9 | Arduino Nano (or clone) | 1 | $3-5 | "Arduino Nano V3 CH340" |
| 10 | SG90 Micro Servo Motor | 1 | $2-3 | "SG90 micro servo 9g" |
| 11 | IR Obstacle Avoidance Sensor (FC-51) | 1 | $1-2 | "IR obstacle avoidance sensor FC-51" |
| 12 | 2-Pin Magnetic Pogo Connector (cable side) | 1 | $1-2 | "magnetic pogo pin connector 2 pin" |
| 13 | Thin Silicone Wire (flexible, 26AWG) | 1 ft | $1 | "silicone wire 26AWG" |
| 14 | Red LED | 1 | $0.25 | "5mm red LED" |
| 15 | Green LED | 1 | $0.25 | "5mm green LED" |
| 16 | 220 ohm Resistors | 2 | $0.25 | "220 ohm resistor pack" |
| 17 | 5V USB Power Supply | 1 | $0 | Any phone charger + USB cable |

**Dock subtotal: ~$9-14**

## Total Budget: ~$37-59

## Optional Extras

| Component | Price | What it adds |
|-----------|-------|-------------|
| 18650 USB Charger (backup) | $5-8 | Charge batteries outside the car |
| 3D-Printed Cybertruck Shell | $10-20 | Tesla body for the car |
| 3D-Printed Supercharger Stall | $2-5 | Miniature Tesla Supercharger housing |
| Copper Tape (conductive adhesive) | $1-2 | Alternative to magnetic connectors |

## What Comes in the 2WD Chassis Kit

- 2x DC gear motors (3-6V)
- 2x Wheels
- 1x Caster wheel (front)
- 1x Acrylic chassis plate
- Mounting screws and standoffs
- Battery box (not used — we use 18650 holder instead)

## Wiring Diagram — Car

```
ESP32-CAM + L298N + TP4056
============================

Power:
  18650 Holder (+) --> TP4056 BAT+ --> L298N 12V input
  18650 Holder (-) --> TP4056 BAT- --> L298N GND
  L298N 5V output  --> ESP32-CAM 5V
  L298N GND        --> ESP32-CAM GND

Charge Port (rear of car):
  Magnetic connector (+) --> TP4056 IN+
  Magnetic connector (-) --> TP4056 IN-

Motor A (Left):
  ESP32-CAM GPIO 14 --> L298N IN1
  ESP32-CAM GPIO 15 --> L298N IN2
  L298N OUT1        --> Left Motor (+)
  L298N OUT2        --> Left Motor (-)

Motor B (Right):
  ESP32-CAM GPIO 13 --> L298N IN3
  ESP32-CAM GPIO  2 --> L298N IN4
  L298N OUT3        --> Right Motor (+)
  L298N OUT4        --> Right Motor (-)

Charging Status (TP4056 to ESP32):
  TP4056 CHRG pin   --> ESP32-CAM GPIO 16 (LOW = charging)
  TP4056 STDBY pin  --> ESP32-CAM GPIO 12 (LOW = full)

Note: Remove the ENA/ENB jumpers on L298N for full speed,
      or connect to PWM pins for variable speed control.
```

## Wiring Diagram — Supercharger Dock

```
Arduino Nano + Servo + IR Sensor
==================================

Power (from USB):
  USB 5V  --> Nano 5V, Servo VCC, IR Sensor VCC
  USB GND --> Nano GND, Servo GND, IR Sensor GND

IR Sensor (detects car):
  FC-51 OUT --> Nano D2

Servo (swings charging cable):
  SG90 Signal --> Nano D9

Charging Cable:
  USB 5V  --> Thin wire through servo arm --> Magnetic connector (+)
  USB GND --> Thin wire through servo arm --> Magnetic connector (-)

Status LEDs:
  Nano D4 --> 220 ohm resistor --> Red LED --> GND   (charging)
  Nano D5 --> 220 ohm resistor --> Green LED --> GND  (complete)

Physical Layout (top view):

  ┌──────────────────────────────┐
  │       SUPERCHARGER DOCK      │
  │                              │
  │   [Nano]        [IR Sensor]  │
  │                    |         │
  │         [Servo]────┘         │
  │          /                   │
  │   cable / (swings out        │
  │        /   to reach car)     │
  │       /                      │
  │  [mag connector]             │
  │         ↕                    │
  │     [CAR parks here]         │
  │                              │
  └──────────────────────────────┘

Servo Arm Positions:
  Retracted (90°)  = cable tucked in, waiting
  Extended  (0°)   = cable swung out to car's charge port

Car Charge Port Location:
  Mount the magnetic connector on the rear-left of the car,
  matching the driver-side charge port on a real Tesla.
```

## ESP32-CAM Pinout Reference

```
                 ┌─────────────┐
                 │   ESP32-CAM  │
                 │  ┌───────┐  │
            3V3 ─┤  │CAMERA │  ├─ 5V
           IO16 ─┤  │       │  ├─ GND
            IO0 ─┤  │       │  ├─ IO12 --> TP4056 STDBY
            GND ─┤  └───────┘  ├─ IO13 --> L298N IN3
            VCC ─┤    [LED]    ├─ IO15 --> L298N IN2
           IO14 ─┤             ├─ IO14 --> L298N IN1
            IO2 ─┤             ├─ IO2  --> L298N IN4
            IO4 ─┤  [SD CARD]  ├─ IO4  (flash LED)
                 └─────────────┘

  IO16 --> TP4056 CHRG (charging status)
  IO12 --> TP4056 STDBY (fully charged status)
```
