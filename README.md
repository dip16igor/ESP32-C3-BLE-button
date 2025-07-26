[English](./README.md) | [Русский](./README.ru.md)

# ESP32-C3 BLE Pressure Emulator

This project turns an ESP32-C3 based board into a BLE peripheral that emulates a pressure sensor. The target pressure value can be controlled both with physical buttons on the device and via Bluetooth Low Energy. The current pressure value changes smoothly and is sent to a connected client via BLE notifications.

## Key Features

- **BLE Service:** Creates a custom BLE service with a single characteristic for control and data reading.
- **Pressure Emulation:** The pressure value smoothly approaches the target value using a low-pass filter, providing a realistic change.
- **Dual Control:**
  - **Physical Buttons:** Two buttons to set positive (`+50.0`) and negative (`-50.0`) target pressures.
  - **BLE Commands:** Send write commands to the characteristic to set the target pressure.
- **Feedback:**
  - **BLE Notifications:** The current pressure value (type `float`, 4 bytes) is sent to subscribers every 50 ms.
  - **LED Indication:** LEDs show the BLE connection status and the current pressure direction (positive/negative).

## Hardware Components

- ESP32-C3 development board (e.g., ESP32-C3 Super Mini).
- 2 push-buttons.
- 1 RGB LED (or 3 separate LEDs: red, green, blue).
- Jumper wires.

## Pinout

| Component                           | GPIO Pin |
| :---------------------------------- | :------- |
| Button 1 (positive pressure)        | `GPIO 0` |
| Button 2 (negative pressure)        | `GPIO 1` |
| Blue LED (connection indicator)     | `GPIO 2` |
| Green LED (positive pressure)       | `GPIO 3` |
| Red LED (negative pressure)         | `GPIO 4` |
| Built-in LED (advertising indicator)| `GPIO 8` |

_Note: Buttons should be connected between the GPIO pin and ground (GND). The code uses the internal pull-up resistor (`INPUT_PULLUP`)._

## Software and Libraries

- **Development Environment:** PlatformIO or Arduino IDE.
- **Library:** `NimBLE-Arduino` (installed automatically via PlatformIO as defined in `platformio.ini`).

## How to Use

### 1. Build and Flash

1.  Open the project in your favorite IDE (e.g., VS Code with PlatformIO).
2.  Compile and upload the firmware to your ESP32-C3 board.

### 2. Interact via BLE

1.  Use any BLE scanner app (e.g., **nRF Connect for Mobile**, **LightBlue**).
2.  Find and connect to the device named `C1-Emulator`.
3.  **Service UUID:** `12345678-1234-1234-1234-123456789abc`
4.  **Characteristic UUID:** `abcdefab-1234-5678-9abc-abcdefabcdef`

### 3. Control the Device

- **Receiving Data:**
  - In the app, find the specified characteristic and subscribe to **Notifications**.
  - You will start receiving 4-byte values (little-endian float) of the current pressure every 50 ms.

- **Sending Commands:**
  - Write a byte to the same characteristic using **Write/Write without response**:
    - `0x01`: Set target pressure to `+40.0`. The green LED will light up.
    - `0x02`: Set target pressure to `-40.0`. The red LED will light up.
    - `0x00` or `0x03`: Set target pressure to `0.0`. The red and green LEDs will turn off.

## LED Logic

- **Built-in LED (GPIO 8):** Blinks when the device is in advertising mode (waiting for connection). Turns off after a client connects.
- **Blue LED (GPIO 2):** Lights up upon a successful BLE client connection.
- **Green LED (GPIO 3):** Lights up when a command for positive pressure is received via BLE.
- **Red LED (GPIO 4):** Lights up when a command for negative pressure is received via BLE.

