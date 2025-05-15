# EPD47 Display System

The EPD47 Display System is a client-server application for remotely managing content on LilyGo EPD47 e-paper displays using ESP32S3 microcontrollers.

---

## Overview

This system allows you to push text and image content from a central server to multiple e-paper displays over WiFi. It is designed for low-power, low-refresh applications such as signage, dashboards, or dynamic labels.

---

## System Components

### Client (ESP32 + EPD47)

The client runs on an ESP32S3 board connected to a LilyGo EPD47 (960×540) e-paper display.

**Responsibilities:**

- Connect to a WiFi network
- Periodically request updates from the server
- Render either text or JPEG images on the e-paper display
- Show status info like connection and update state

### Server

A lightweight HTTP server that:

- Listens on port `3000`
- Responds with content in JSON format for the display to render

**JSON Format:**

```json
{
  "type": "text" | "image",
  "data": "String content or base64-encoded image"
}

Hardware Requirements
ESP32S3 Development Board

LilyGo EPD47 e-paper display (960×540 resolution)

5V power supply for ESP32

WiFi network with internet access (optional)

Software Requirements
Arduino IDE Settings
Board: ESP32S3 Dev Module

Flash Size: 16MB (128Mb)

PSRAM: OPI PSRAM

Required Libraries
Install the following libraries via the Arduino Library Manager or include them manually:

WiFi.h – Network connectivity

HTTPClient.h – HTTP requests

epd_driver.h – Display driver

firasans.h – Font rendering

JPEGDecoder.h – JPEG image decoding

Installation
Install the latest Arduino IDE.

Install the ESP32 board package (via Boards Manager).

Install the required libraries listed above.

Clone or download this repository.

Open the client/ project in Arduino IDE.

Edit the WiFi and server configuration:

const char *ssid = "YourNetworkName";
const char *password = "YourPassword";
const char *server_url = "http://your-server-address:3000/endpoint";

Upload the code to your ESP32S3.

Reboot the device to begin polling the server.

Server API
The server should provide a JSON response containing either text or image content.

Example Response (Text):

{
  "type": "text",
  "data": "Hello from the server!"
}
Example Response (Image):

{
  "type": "image",
  "data": "/9j/4AAQSkZJRgABAQEAYABgAAD..." // Base64-encoded JPEG
}
Project Structure

EPD47-Display-System/
├── client/           # Arduino sketch and ESP32 code
├── server/           # Node.js, Python, or other server implementation
├── assets/           # Fonts, images, test content
└── README.md
