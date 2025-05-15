# E-Paper Display System - Quick Start Guide

## Package Contents

This simplified package contains all the essential components needed to test and present the LilyGo-EPD47 e-paper display system:

```
EPD47_Display_System/
├── README.md                 # Main documentation
├── setup_guide.md            # Detailed setup instructions
├── QUICK_START.md            # This quick start guide
├── server_display/           # Direct mode firmware
│   └── server_display.ino    # ESP32 hosts its own web server
├── client_display/           # Client mode firmware
│   └── client_display.ino    # ESP32 connects to PC server
└── pc_server/                # PC server application
    ├── package.json          # Node.js dependencies
    ├── server.js             # Server application
    └── public/               # Web interface
        └── index.html        # Control interface
```

## Quick Setup Steps

### Option 1: Direct Mode (Standalone ESP32)

1. Open `server_display/server_display.ino` in Arduino IDE
2. Update WiFi credentials:
   ```cpp
   const char *ssid = "YOUR_WIFI_SSID";
   const char *password = "YOUR_WIFI_PASSWORD";
   ```
3. Upload to ESP32
4. Note the IP address displayed on the e-paper screen
5. Access the web interface by entering the IP address in a browser

### Option 2: PC Server Mode

1. Set up the PC server:
   ```
   cd pc_server
   npm install
   node server.js
   ```

2. Open `client_display/client_display.ino` in Arduino IDE
3. Update WiFi credentials and server address:
   ```cpp
   const char *ssid = "YOUR_WIFI_SSID";
   const char *password = "YOUR_WIFI_PASSWORD";
   const char *serverAddress = "http://YOUR_PC_IP:3000";
   ```
4. Upload to ESP32
5. Access the web interface at `http://localhost:3000`
6. Register the ESP32 device using its IP address

## Testing Checklist

- [ ] ESP32 connects to WiFi successfully
- [ ] Web interface loads correctly
- [ ] Text display works
- [ ] Image upload and display works
- [ ] Clear screen function works
- [ ] Multiple devices can be registered (PC server mode)

## Presentation Tips

1. **Prepare Demo Content**
   - Have sample text ready to display
   - Prepare a few JPEG images sized appropriately for the display (960×540 pixels)

2. **Highlight Key Features**
   - Low power consumption (display retains content without power)
   - Wireless control from any device with a browser
   - Multiple control options (direct or via PC server)
   - Support for both text and images

3. **Demonstrate Both Modes**
   - Show direct mode for simplicity
   - Show PC server mode for multi-device support

4. **Interactive Demo**
   - Let audience members send content to the display from their own devices
   - Show how multiple displays can be controlled from a single interface

## Automated Setup Tools

This package includes automated tools to simplify the setup process:

1. **Setup Script** (`setup.bat`)
   - Automatically checks for Node.js installation
   - Installs all required dependencies
   - Creates necessary directories
   - Run this script first to set up the PC server

2. **Verification Script** (`verify_setup.bat`)
   - Checks if all components are correctly installed
   - Verifies the presence of required files
   - Confirms the system is ready for testing

3. **Sample Content**
   - A sample SVG image is included in `pc_server/public/sample_image.svg`
   - Use this to test the image display functionality

## Troubleshooting

- If the ESP32 fails to connect to WiFi, check credentials and signal strength
- If the PC server can't be accessed, check firewall settings
- If images don't display correctly, ensure they're in JPEG format and not too large
- If the ESP32 client can't connect to the server, verify the server address is correct
- If you encounter issues with the setup, run the verification script to check your installation