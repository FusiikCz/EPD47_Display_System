# EPD47 Display System

The EPD47 Display System is a client-server application for remotely managing content on LilyGo EPD47 e-paper displays using ESP32S3 microcontrollers.

---

## Overview

This system allows you to push text and image content from a central server to multiple e-paper displays over WiFi. It is designed for low-power, low-refresh applications such as signage, dashboards, or dynamic labels.

---

## System Components

### Client (ESP32 + EPD47)

The client runs on an ESP32S3 board connected to a LilyGo EPD47 (960×540) e-paper display.

**Client responsibilities:**

- Connect to a WiFi network
- Periodically request updates from the server
- Render either text or JPEG images on the e-paper display
- Display device status and update timestamps

### Server

The server component provides content updates to clients via HTTP.

**Server responsibilities:**

- Host an HTTP server on port `3000`
- Provide content via a REST endpoint in JSON format
- Support both text and image content

**JSON Format Example:**

```json
{
  "type": "text",
  "data": "Welcome to EPD47 Display System"
}
````

```json
{
  "type": "image",
  "data": "base64-encoded JPEG string"
}
```

-----

## Hardware Requirements

  * ESP32S3 development board
  * LilyGo EPD47 e-paper display (960×540 pixels)
  * 5V power supply for the ESP32 board
  * WiFi access point (2.4 GHz)

-----

## Software Requirements

### Arduino IDE Configuration

Ensure the following settings in the Arduino IDE:

  * **Board**: ESP32S3 Dev Module
  * **Flash Size**: 16MB (128Mb)
  * **PSRAM**: OPI PSRAM

### Required Libraries

Install these libraries via the Arduino Library Manager:

  * `WiFi.h` - For network connectivity
  * `HTTPClient.h` - For making HTTP requests
  * `epd_driver.h` - For controlling the e-paper display
  * `firasans.h` - For font rendering
  * `JPEGDecoder.h` - For decoding JPEG images

-----

## Installation

1.  Install the [Arduino IDE](https://www.arduino.cc/en/software).

2.  Install the ESP32 board support via the Boards Manager.

3.  Install all required libraries.

4.  Clone this repository:

    ```sh
    git clone [https://https://github.com/FusiikCz/EPD47_Display_System.git](https://https://github.com/FusiikCz/EPD47_Display_System.git)
    ```

5.  Open the `client/` folder in Arduino IDE.

6.  Edit the WiFi and server configuration:

    ```cpp
    const char *ssid = "YourNetworkName";
    const char *password = "YourPassword";
    const char *server_url = "http://your-server-address:3000/endpoint";
    ```

7.  Upload the sketch to your ESP32S3 board.

8.  Power on the device and verify the display content.

-----

## Server API Specification

The client polls the server for display updates. The server should respond with a JSON payload like the following:

### Text Content Example

```json
{
  "type": "text",
  "data": "Now Showing: Meeting at 3 PM"
}
```

### Image Content Example

```json
{
  "type": "image",
  "data": "/9j/4AAQSkZJRgABAQEASABIAAD..."
}
```

-----

## Directory Structure

```
epd47-display-system/
├── client/           # ESP32 client code (Arduino sketch)
├── server/           # Server code (Node.js or Python)
├── assets/           # Sample images, fonts, or test files
└── README.md         # Project documentation
```

-----

## License

This project is licensed under the MIT License.

```
```
