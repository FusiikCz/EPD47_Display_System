/**
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-04-05
 * @note      Arduino Setting
 *            Tools ->
 *                  Board:"ESP32S3 Dev Module"
 *                  USB CDC On Boot:"Enable"
 *                  USB DFU On Boot:"Disable"
 *                  Flash Size : "16MB(128Mb)"
 *                  Flash Mode"QIO 80MHz
 *                  Partition Scheme:"16M Flash(3M APP/9.9MB FATFS)"
 *                  PSRAM:"OPI PSRAM"
 *                  Upload Mode:"UART0/Hardware CDC"
 *                  USB Mode:"Hardware CDC and JTAG"
 *  
 */

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include "epd_driver.h"
#include "libjpeg/libjpeg.h"
#include "firasans.h"
#include "czech_font.h"  // Add your Czech-supporting font
#include <FS.h>
#include <FFat.h>
#include "utilities.h"

// WiFi credentials - replace with your network details
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";

// Web server on port 80
WebServer server(80);

// Display buffer
uint8_t *framebuffer;
char buf[128];

// Display areas for status messages
const Rect_t statusArea = {
    .x = 0,
    .y = 480,
    .width = 960,
    .height = 60,
};

const Rect_t mainArea = {
    .x = 0,
    .y = 0,
    .width = 960,
    .height = 480,
};

// Function prototypes
void displayTextOnScreen(String text);
void displayImageOnScreen(uint8_t* imageData, size_t imageSize);
void updateStatusMessage(const char* message);
void handleRoot();
void handleDisplayText();
void handleUploadImage();
void handleClear();
void handleNotFound();

void setup() {
    Serial.begin(115200);
    
    // Initialize the display
    epd_init();
    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("Error: Could not allocate memory for framebuffer");
        return;
    }
    
    // Clear the display
    epd_poweron();
    epd_clear();
    
    // Initialize file system
    if (!FFat.begin()) {
        Serial.println("FFat Mount Failed");
    }
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("");
    Serial.print("Connected to WiFi. IP address: ");
    Serial.println(WiFi.localIP());
    
    // Display IP address on screen
    sprintf(buf, "Server started at: http://%s", WiFi.localIP().toString().c_str());
    updateStatusMessage(buf);
    
    // Set up web server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/display-text", HTTP_POST, handleDisplayText);
    server.on("/upload-image", HTTP_POST, []() {
        server.send(200, "text/plain", "Image upload complete");
    }, handleUploadImage);
    server.on("/clear", HTTP_POST, handleClear);
    
    // Set up web server routes
    server.enableCORS(true);
    server.onNotFound(handleNotFound);
    
    // Start the server
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}

// Display text on the e-paper
void displayTextOnScreen(String text) {
    epd_poweron();
    epd_clear_area(mainArea);
    
    int32_t cursor_x = mainArea.x + 20;
    int32_t cursor_y = mainArea.y + 40;
    
    // Split text by newlines and display each line
    int startPos = 0;
    int endPos = text.indexOf('\n');
    
    while (startPos < text.length()) {
        if (endPos == -1) {
            endPos = text.length();
        }
        
        String line = text.substring(startPos, endPos);
        // Use the Czech font instead of FiraSans
        writeln((GFXfont *)&CzechFont, line.c_str(), &cursor_x, &cursor_y, NULL);
        
        startPos = endPos + 1;
        endPos = text.indexOf('\n', startPos);
        cursor_x = mainArea.x + 20;
    }
    
    epd_poweroff();
}

// Display image on the e-paper with optimized scaling
void displayImageOnScreen(uint8_t* imageData, size_t imageSize) {
    epd_poweron();
    epd_clear_area(mainArea);
    
    // Decode and display JPEG image
    int jpegWidth, jpegHeight;
    uint8_t* decoded = (uint8_t*)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT, MALLOC_CAP_SPIRAM);
    
    if (decoded) {
        // Get image dimensions first
        if (decode_jpeg(&jpegWidth, &jpegHeight, imageData, imageSize, NULL, 0)) {
            // Calculate optimal scaling to maximize display area usage
            float scaleX = (float)mainArea.width / jpegWidth;
            float scaleY = (float)mainArea.height / jpegHeight;
            float scale = min(scaleX, scaleY); // Use the smaller scale to fit the screen
            
            // Calculate new dimensions
            int newWidth = jpegWidth * scale;
            int newHeight = jpegHeight * scale;
            
            // Calculate centered position
            int x = mainArea.x + (mainArea.width - newWidth) / 2;
            int y = mainArea.y + (mainArea.height - newHeight) / 2;
            
            // Ensure coordinates are valid
            if (x < 0) x = 0;
            if (y < 0) y = 0;
            
            // Decode and display with scaling
            if (decode_jpeg(&jpegWidth, &jpegHeight, imageData, imageSize, decoded, EPD_WIDTH * EPD_HEIGHT)) {
                // Create a rectangle with the new dimensions
                Rect_t imageRect = {
                    .x = x,
                    .y = y,
                    .width = newWidth,
                    .height = newHeight
                };
                
                // Draw the image with scaling
                epd_draw_grayscale_image(imageRect, decoded);
            }
        }
        heap_caps_free(decoded);
    }
    
    epd_poweroff();
}

// Update status message at the bottom of the screen
void updateStatusMessage(const char* message) {
    epd_poweron();
    epd_clear_area(statusArea);
    
    int32_t cursor_x = statusArea.x + 10;
    int32_t cursor_y = statusArea.y + 30;
    
    // Also use the Czech font for status messages
    writeln((GFXfont *)&CzechFont, message, &cursor_x, &cursor_y, NULL);
    
    epd_poweroff();
}

// Handle root page
void handleRoot() {
    String html = "<!DOCTYPE html>\n"
                 "<html>\n"
                 "<head>\n"
                 "  <title>E-Paper Display Control</title>\n"
                 "  <meta name='viewport' content='width=device-width, initial-scale=1'>\n"
                 "  <style>\n"
                 "    body { font-family: Arial, sans-serif; margin: 20px; }\n"
                 "    h1 { color: #333; }\n"
                 "    .container { max-width: 800px; margin: 0 auto; }\n"
                 "    .section { background: #f5f5f5; padding: 20px; margin-bottom: 20px; border-radius: 5px; }\n"
                 "    textarea { width: 100%; height: 100px; margin-bottom: 10px; }\n"
                 "    button { background: #4CAF50; color: white; border: none; padding: 10px 15px; cursor: pointer; }\n"
                 "    button:hover { background: #45a049; }\n"
                 "    .clear-btn { background: #f44336; }\n"
                 "    .clear-btn:hover { background: #d32f2f; }\n"
                 "  </style>\n"
                 "</head>\n"
                 "<body>\n"
                 "  <div class='container'>\n"
                 "    <h1>E-Paper Display Control</h1>\n"
                 "    \n"
                 "    <div class='section'>\n"
                 "      <h2>Display Text</h2>\n"
                 "      <form action='/display-text' method='post'>\n"
                 "        <textarea name='text' placeholder='Enter text to display...'></textarea><br>\n"
                 "        <button type='submit'>Send Text</button>\n"
                 "      </form>\n"
                 "    </div>\n"
                 "    \n"
                 "    <div class='section'>\n"
                 "      <h2>Upload Image</h2>\n"
                 "      <form action='/upload-image' method='post' enctype='multipart/form-data'>\n"
                 "        <input type='file' name='image' accept='image/jpeg'><br><br>\n"
                 "        <button type='submit'>Upload Image</button>\n"
                 "      </form>\n"
                 "    </div>\n"
                 "    \n"
                 "    <div class='section'>\n"
                 "      <h2>Clear Screen</h2>\n"
                 "      <form action='/clear' method='post'>\n"
                 "        <button type='submit' class='clear-btn'>Clear Screen</button>\n"
                 "      </form>\n"
                 "    </div>\n"
                 "  </div>\n"
                 "</body>\n"
                 "</html>";
    
    server.send(200, "text/html", html);
}

// Handle display text request
void handleDisplayText() {
    if (server.hasArg("text")) {
        String text = server.arg("text");
        displayTextOnScreen(text);
        server.send(200, "text/plain", "Text displayed successfully");
        
        // Update status message
        updateStatusMessage("Text displayed successfully");
    } else {
        server.send(400, "text/plain", "No text provided");
    }
}

// Handle image upload
void handleUploadImage() {
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        Serial.println("Receiving image...");
        updateStatusMessage("Receiving image...");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        // Process the received data
        if (upload.currentSize > 0) {
            displayImageOnScreen(upload.buf, upload.currentSize);
            updateStatusMessage("Image displayed successfully");
        }
    }
}

// Handle clear screen request
void handleClear() {
    epd_poweron();
    epd_clear();
    epd_poweroff();
    
    updateStatusMessage("Display cleared");
    server.send(200, "text/plain", "Display cleared");
}

// Handle 404 Not Found
void handleNotFound() {
    server.send(404, "text/plain", "404: Not found");
}