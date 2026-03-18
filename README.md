# Maxwell

An ESP32-CAM FPV car with a Tesla-inspired auto-plug Supercharger dock.

Drive with a PS5 controller from your Mac or PC while watching a live camera feed. When the battery runs low, park near the Supercharger — a servo arm swings out and plugs in a magnetic charging cable automatically.

## Features

- **Live FPV Camera** — 640x480 MJPEG stream from the ESP32-CAM to any browser
- **PS5 DualSense Controller** — drive from your computer via browser Gamepad API
- **Keyboard & Touch Controls** — arrow keys, WASD, or on-screen buttons
- **Auto-Plug Supercharger Dock** — IR sensor detects the car, servo arm swings a magnetic cable to the charge port, charges the 18650s, retracts when done
- **Lithium-Ion Powered** — 2x 18650 rechargeable cells (same chemistry as Tesla)
- **Battery Status** — web interface shows charging state with animated bar
- **Motor Safety** — motors auto-disable while charging

## Build Phases

| Phase | What You Build | Folder |
|-------|---------------|--------|
| 1 | Motor test — verify wiring works | `phase1_motor_control/` |
| 2 | Camera test — verify WiFi stream | `phase2_camera/` |
| 3 | Full FPV car — camera + PS5 controller | `phase3_full_car/` |
| 4 | Supercharger dock — auto-plug charging | `phase4_supercharger/` |

## Architecture

```
                    WiFi
  [Computer]  <────────── [ESP32-CAM on car]
   PS5 controller           |-- OV2640 camera
   Web browser              |-- L298N motor driver
                            |-- 2x DC motors
                            |-- TP4056 charge module
                            |-- Magnetic charge port (rear)

  [Supercharger Dock]
   Arduino Nano             |-- IR sensor (detects car)
   5V USB powered           |-- SG90 servo (swings cable arm)
                            |-- Magnetic connector on cable
                            |-- Red LED (charging)
                            |-- Green LED (fully charged)
```

## How the Supercharger Works

```
  1. Car parks near dock
  2. IR sensor detects the car
  3. Arduino Nano triggers the SG90 servo
  4. Servo arm swings magnetic cable to car's charge port
  5. Magnetic connector snaps onto contact pads
  6. TP4056 charges the 18650 batteries
  7. Red LED = charging, Green LED = full
  8. When full, servo retracts the cable automatically
```

```
  Side view:

  [SUPERCHARGER]          [CAR]
       |                    |
       |    servo arm       |
       |   ╭──────────╮    |
       |   │  cable    │--->|=| charge port
       |   ╰──────────╯    |
       |                    |
   [IR sensor] -----> detects car
```

## Parts List (~$45-60)

See [PARTS_LIST.md](PARTS_LIST.md) for the full shopping list with search keywords.

### Car

| Component | Qty | Est. Price |
|-----------|-----|-----------|
| ESP32-CAM (with OV2640 + USB programmer) | 1 | $6-10 |
| 2WD Robot Car Chassis Kit | 1 | $10-15 |
| L298N Motor Driver Module | 1 | $3-5 |
| 18650 Batteries (3.7V rechargeable) | 2 | $4-6 |
| 18650 Battery Holder (2-slot w/ switch) | 1 | $1-2 |
| TP4056 Charging Module (USB-C) | 1 | $1-2 |
| Magnetic Pogo Connector (2-pin, car side) | 1 | $1-2 |
| Jumper Wires (F-F, M-F) | 1 pack | $2-3 |

### Supercharger Dock

| Component | Qty | Est. Price |
|-----------|-----|-----------|
| Arduino Nano (or clone) | 1 | $3-5 |
| SG90 Micro Servo | 1 | $2-3 |
| IR Obstacle Sensor (FC-51) | 1 | $1-2 |
| Magnetic Pogo Connector (2-pin, cable side) | 1 | $1-2 |
| Thin Silicone Wire (flexible) | 1 ft | $1 |
| Red LED + Green LED | 2 | $0.50 |
| 220 ohm Resistors | 2 | $0.25 |
| 5V USB Power Supply | 1 | You have one |

### Optional

| Component | Price | What it adds |
|-----------|-------|-------------|
| 18650 USB Charger (backup) | $5-8 | Charge batteries outside the car |
| 3D-Printed Tesla Shell | $10-20 | Cybertruck body |
| 3D-Printed Supercharger Stall | $2-5 | Miniature Tesla Supercharger look |

## Quick Start

### 1. Install Arduino IDE

Download from [arduino.cc/en/software](https://www.arduino.cc/en/software)

### 2. Add ESP32 Board Support

1. Open Arduino IDE -> **Settings** -> **Additional Board Manager URLs**
2. Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Go to **Tools -> Board -> Board Manager** -> search "esp32" -> Install **esp32 by Espressif Systems**
4. Select board: **AI Thinker ESP32-CAM**

### 3. Upload Phase 1 (Motor Test)

1. Open `phase1_motor_control/phase1_motor_control.ino`
2. Connect ESP32-CAM via USB programmer
3. Select port under **Tools -> Port**
4. Upload — motors should cycle through directions

### 4. Upload Phase 3 (Full FPV Car)

1. Open `phase3_full_car/phase3_full_car.ino`
2. Change `YOUR_WIFI_NAME` and `YOUR_WIFI_PASSWORD`
3. Upload, open Serial Monitor at 115200 baud
4. Open the IP address shown in your browser
5. Connect PS5 controller to your computer and drive

### 5. Build the Supercharger

1. Wire up the Arduino Nano + servo + IR sensor (see [PARTS_LIST.md](PARTS_LIST.md))
2. Open `phase4_supercharger/phase4_supercharger.ino`
3. Upload to the Arduino Nano
4. Power the dock via USB
5. Park Maxwell near the dock and watch it charge

## Controls

| Input | Forward | Backward | Left | Right | Stop |
|-------|---------|----------|------|-------|------|
| PS5 Controller | Left stick up | Left stick down | Left stick left | Left stick right | X button |
| PS5 D-pad | Up | Down | Left | Right | X button |
| Keyboard | W / Arrow Up | S / Arrow Down | A / Arrow Left | D / Arrow Right | Space |

## License

MIT — build it, mod it, share it.
