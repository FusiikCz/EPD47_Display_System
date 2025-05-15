# Quick Setup Guide for E-Paper Display System

## Hardware Requirements
- LilyGo EPD47 E-Paper Display with ESP32
- USB cable for programming
- Computer for running the PC server
- Mobile device for testing the web interface

## Setup Checklist

### 1. ESP32 Setup
- [ ] Install Arduino IDE and ESP32 board support
- [ ] Select correct board settings (see comments in .ino files)
- [ ] Configure WiFi credentials in the firmware
- [ ] Upload firmware to ESP32

### 2. PC Server Setup
- [ ] Install Node.js on your computer
- [ ] Install required npm packages
- [ ] Start the server
- [ ] Access the web interface

### 3. Testing
- [ ] Verify ESP32 connects to WiFi
- [ ] Register device in web interface
- [ ] Send test text to display
- [ ] Send test image to display
- [ ] Clear display

## Presentation Flow

1. **Introduction**
   - Show the hardware components
   - Explain the system architecture

2. **Direct Mode Demo**
   - Connect to ESP32's web server
   - Send text and images directly

3. **PC Server Mode Demo**
   - Show the PC server web interface
   - Register the ESP32 device
   - Send content from multiple devices

4. **Features Highlight**
   - Multi-device support
   - Text and image display
   - Low power consumption
   - Wireless control

5. **Q&A**

## Sample Content for Demo

### Text Examples
- "Hello World! This is the LilyGo EPD47 E-Paper Display"
- "E-Paper displays maintain their content without power"
- "This display has a resolution of 960×540 pixels"

### Image Suggestions
- Company logo
- Product diagrams
- QR code linking to more information