# EPD47 Display System

## Overview
The EPD47 Display System is a client-server solution for remotely managing content on e-paper displays. The system uses ESP32S3 microcontrollers with LilyGo EPD47 e-paper displays to show text and images sent from a central server.

## System Components

### Client Display (ESP32)
The client component runs on ESP32S3 microcontrollers connected to EPD47 e-paper displays. Each client:
- Connects to WiFi
- Polls a server for content updates
- Renders text and images on the e-paper display
- Provides status updates on the display

### Server Component
The server component should:
- Host an HTTP server on port 3000
- Provide endpoints for content polling
- Serve content in JSON format with type and data fields

## Hardware Requirements

- ESP32S3 Development Board
- LilyGo EPD47 E-Paper Display (960×540 pixels)
- Power supply for the ESP32
- WiFi network for connectivity

## Software Requirements

### Arduino IDE Settings
- Board: "ESP32S3 Dev Module"
- Flash Size: "16MB(128Mb)"
- PSRAM: "OPI PSRAM"

### Required Libraries
- WiFi.h - For network connectivity
- HTTPClient.h - For HTTP requests
- epd_driver.h - For controlling the e-paper display
- firasans.h - For font rendering
- JPEGDecoder.h - For decoding JPEG images

## Installation and Setup

1. Install the Arduino IDE and ESP32 board support
2. Install required libraries
3. Configure WiFi settings in the client code
4. Update the server address to match your setup
5. Upload the code to your ESP32

## Configuration

### WiFi Settings
```cpp
const char *ssid = "YourNetworkName";
const char *password = "YourPassword";
```

