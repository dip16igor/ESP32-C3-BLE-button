[English](./README.md) | [Русский](./README.ru.md)

# ESP32-C3 BLE Pressure Service Emulator

This project is a BLE service emulator for a pressure control device, running on an ESP32-C3. It simulates pressure changes based on physical button presses or BLE commands and notifies connected clients.

## Features
*   **BLE Service**: Implements a custom BLE service with a single characteristic.
*   **Read/Write/Notify**: The characteristic supports reading the current pressure, writing control commands, and sending notifications.
*   **Pressure Simulation**: Smoothly simulates pressure changes towards a target value using a low-pass filter.
*   **Dual Control**: Can be controlled via two physical buttons or by writing commands to the BLE characteristic.
*   **Status LEDs**: Uses multiple LEDs to indicate power, BLE connection status, and operational state (positive/negative pressure).

## Hardware
*   ESP32-C3 Super Mini (or similar)
*   LEDs (Built-in, Blue, Green, Red)
*   Two push-buttons

## BLE Protocol

- **Service UUID**: `12345678-1234-1234-1234-123456789abc`
- **Characteristic UUID**: `abcdefab-1234-5678-9abc-abcdefabcdef`

### Write Commands (single byte)
- `1`: Set target pressure to `+40.0`.
- `2`: Set target pressure to `-40.0`.
- `0` or `3`: Set target pressure to `0.0`.