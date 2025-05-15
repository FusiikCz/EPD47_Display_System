# E-Paper Display System - Installation Guide

This guide provides step-by-step instructions for installing and configuring the E-Paper Display System.

## Prerequisites

- Node.js installed (version 12.x or higher)
- Arduino IDE installed with ESP32 board support
- LilyGo EPD47 E-Paper Display with ESP32
- USB cable for programming the ESP32
- WiFi network for connecting the ESP32

## Installation Steps

### 1. Set Up the PC Server

The easiest way to set up the PC server is to use the provided setup script:

1. Open a command prompt in the EPD47_Display_System directory
2. Run the setup script:
   ```
   setup.bat
   ```
3. The script will check for Node.js and install all required dependencies

### 2. Verify the Installation

After running the setup script, verify that everything is installed correctly:

1. Run the verification script:
   ```
   verify_setup.bat
   ```
2. The script will check for all required components and confirm if the system is ready

### 3. Configure and Upload the ESP32 Firmware

Choose one of the following modes:

#### Direct Mode (Standalone ESP32)

1. Open `server_display/server_display.ino` in Arduino IDE
2. Update WiFi credentials:
   ```cpp
   const char *ssid = "YOUR_WIFI_SSID";
   const char *password = "YOUR_WIFI_PASSWORD";
   ```
3. Upload to ESP32
4. Note the IP address displayed on the e-paper screen

#### PC Server Mode

1. Open `client_display/client_display.ino` in Arduino IDE
2. Update WiFi credentials and server address:
   ```cpp
   const char *ssid = "YOUR_WIFI_SSID";
   const char *password = "YOUR_WIFI_PASSWORD";
   const char *serverAddress = "http://YOUR_PC_IP:3000";
   ```
3. Upload to ESP32

### 4. Start the Server

1. Navigate to the pc_server directory:
   ```
   cd pc_server
   ```
2. Start the server:
   ```
   node server.js
   ```
3. The server will start on port 3000

### 5. Access the Web Interface

- For Direct Mode: Enter the ESP32's IP address in a browser
- For PC Server Mode: Access `http://localhost:3000` in a browser

## Testing

Use the included sample image to test the display functionality:

1. Access the web interface
2. For PC Server Mode, register your ESP32 device using its IP address
3. Try sending text to the display
4. Upload the sample image (`pc_server/public/sample_image.svg`) to test image display
5. Test the clear screen function

## Next Steps

Refer to the following documents for more information:

- `README.md` - Overview and detailed usage instructions
- `QUICK_START.md` - Quick start guide and presentation tips
- `setup_guide.md` - Detailed setup instructions