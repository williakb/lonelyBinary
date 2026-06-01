# ESP32-S3 WiFi LED Controller

A simple ESP32-S3 project using ESP-IDF, FreeRTOS, WiFi, and WS2811/WS2812 LED control.

Current functionality:

* ESP32-S3 onboard RGB LED control
* External WS2811 12V addressable LED strip control
* WiFi station mode connection
* Embedded HTTP web server
* Browser-based LED control
* Multiple FreeRTOS tasks running concurrently

---

# Hardware

## MCU

* ESP32-S3-N16R8
* 16MB Flash
* 8MB PSRAM

## LED Strip

* ALITOVE WS2811 Addressable 12V RGB Strip
* 9 addressable sections (3 LEDs per section)

## Wiring

### WS2811 Strip

| Strip Wire | Function |
| ---------- | -------- |
| Red        | +12V     |
| White      | GND      |
| Green/Teal | DATA     |

### ESP32 Connections

| ESP32  | LED Strip |
| ------ | --------- |
| GPIO49 | DATA IN   |
| GND    | GND       |

### Power

* LED strip powered from external 12V supply
* ESP32 powered via USB
* Grounds connected together

---

# Features

## FreeRTOS Tasks

The project currently uses multiple FreeRTOS tasks:

* WiFi management
* Onboard RGB status LED
* External LED strip animation
* HTTP web server handling

## Web Server

The ESP32 runs a lightweight HTTP server.

Current endpoints:

| Endpoint | Action         |
| -------- | -------------- |
| /on      | Turn strip on  |
| /off     | Turn strip off |

Example:

http://192.168.x.x/on

http://192.168.x.x/off

---

# ESP-IDF Setup

## ESP-IDF Version

* ESP-IDF v6.0.1

## Build Environment

Developed on:

* Ubuntu 24.04 LTS VM
* VS Code
* ESP-IDF extension

---

# Build Instructions

## Load ESP-IDF Environment

```bash
source ~/.espressif/v6.0.1/esp-idf/export.sh
```

## Build

```bash
idf.py build
```

## Flash + Monitor

```bash
idf.py flash monitor
```

Exit monitor with:

```text
Ctrl + ]
```

---

# Project Structure

```text
blink_test/
├── main/
│   ├── main.c
│   └── CMakeLists.txt
├── CMakeLists.txt
├── sdkconfig
└── README.md
```

---

# Current Concepts Demonstrated

* ESP-IDF project structure
* FreeRTOS task scheduling
* RMT peripheral usage
* WS2811/WS2812 LED control
* WiFi station mode
* Event handlers
* HTTP server
* Concurrent task execution
* Embedded web APIs

---

# Future Ideas

* JSON REST API
* Web UI dashboard
* WebSocket real-time control
* Audio FFT reactive LEDs
* Bluetooth control
* LED matrix animations
* IMU integration
* Multiplayer handheld gaming
* OTA firmware updates
* MQTT/IoT integration

---

# Notes

* WS2811 strips often control LEDs in groups of 3.
* Some 12V WS2811 strips may require a 5V logic level shifter for reliable operation.
* A 330Ω resistor on the DATA line is recommended.
* A large electrolytic capacitor across the 12V rail is recommended for power stability.

---

# License

MIT License
