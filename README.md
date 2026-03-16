# Maxwell

An ESP32-CAM FPV car with PS5 controller support and a self-charging dock inspired by Tesla's Supercharger.

Drive the car from your computer while watching a live camera feed — controlled with a PS5 DualSense controller, keyboard, or on-screen buttons.

## Features

- Live camera streaming (MJPEG over WiFi)
- PS5 DualSense controller support (via browser Gamepad API)
- Keyboard controls (arrow keys / WASD)
- On-screen touch controls (mobile-friendly)
- Charging dock with pogo pin contacts
- Battery status monitoring on the web interface
- Motors auto-disable while charging (safety)

## Build Phases

| Phase | Description | Folder |
|-------|-------------|--------|
| 1 | Motor test — verify wiring works | `phase1_motor_control/` |
| 2 | Camera test — verify WiFi stream | `phase2_camera/` |
| 3 | Full FPV car — camera + PS5 controller | `phase3_full_car/` |
| 4 | Charging dock — drive in and charge | `phase4_charging_dock/` |

## Parts List (~$35-55)

| Component | Qty | Est. Price |
|-----------|-----|-----------|
| ESP32-CAM (with OV2640 camera + USB board) | 1 | $6-10 |
| 2WD Robot Car Chassis Kit | 1 | $10-15 |
| L298N Motor Driver Module | 1 | $3-5 |
| 18650 Batteries (rechargeable 3.7V) | 2 | $4-6 |
| 18650 Battery Holder (2-slot w/ switch) | 1 | $1-2 |
| Jumper Wires (F-F, M-F) | 1 pack | $2-3 |
| TP4056 Charging Module (USB-C) | 1 | $1-2 |
| Pogo Pins (spring-loaded, 2-pin) | 1 pair | $1-2 |
| Copper Tape (adhesive, conductive) | 1 roll | $1-2 |
| 18650 USB Charger (backup/initial charge) | 1 | $5-8 |

Full wiring diagrams are in [PARTS_LIST.md](PARTS_LIST.md).

## Quick Start

1. Install the [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support: `File > Preferences > Additional Board URLs` and add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Select board: `AI Thinker ESP32-CAM`
4. Update WiFi credentials in the `.ino` file
5. Upload and open the Serial Monitor to get the car's IP address
6. Open that IP in your browser and drive

## Controls

| Input | Forward | Backward | Left | Right | Stop |
|-------|---------|----------|------|-------|------|
| PS5 Controller | Left stick up / D-pad up | Left stick down / D-pad down | Left stick left / D-pad left | Left stick right / D-pad right | X button |
| Keyboard | W / Arrow Up | S / Arrow Down | A / Arrow Left | D / Arrow Right | Space |

## Charging Dock

The dock uses spring-loaded pogo pins that contact copper tape pads on the rear of the car. When docked:

- The TP4056 module charges the batteries automatically
- The web interface shows charging status (pulsing orange = charging, green = full)
- Motors are disabled for safety

See [PARTS_LIST.md](PARTS_LIST.md) for the full dock wiring diagram.

## License

MIT
