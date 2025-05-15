#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include "epd_driver.h"
#include "firasans.h"
// Include your new Czech-supporting font
#include "czech_font.h"  // You'll need to create or obtain this file
#include "JPEGDecoder.h"

const char *ssid = "Zdenalovo";
const char *password = "123456789";
const char *serverAddress = "http://192.168.137.1:3000";
const char *pollEndpoint = "/poll-content";
const char *registerEndpoint = "/register-device";

uint8_t *epd_fb = NULL;
uint8_t *framebuffer;

const Rect_t statusArea = {0, 480, 960, 60};
const Rect_t mainArea = {0, 0, 960, 480};

enum ContentType { NONE, TEXT, IMAGE, CLEAR, PROCESSED_IMAGE };
struct Content {
  ContentType type;
  String data;
  String textSize;
};

void updateStatusMessage(const char* message) {
  Serial.println(message);
  epd_poweron();
  epd_clear_area(statusArea);
  int32_t x = statusArea.x + 10, y = statusArea.y + 30;
  // Use the Czech font instead of FiraSans
  writeln((GFXfont *)&CzechFont, message, &x, &y, NULL);
  epd_poweroff();
}

void displayTextOnScreen(String text, String textSize) {
  epd_poweron();
  epd_clear_area(mainArea);
  
  // Debug output
  Serial.print("Displaying text with size: ");
  Serial.println(textSize);
  
  // Choose spacing based on textSize parameter
  int lineSpacing = 40; // Default medium spacing
  int fontSize = 20;    // For future font size implementation
  
  if (textSize == "small") {
    lineSpacing = 25;
    fontSize = 16;
    Serial.println("Using small text settings");
  } else if (textSize == "large") {
    lineSpacing = 60;
    fontSize = 24;
    Serial.println("Using large text settings");
  } else {
    Serial.println("Using medium text settings");
  }
  
  // Split text by newlines and display each line
  int currentLine = 0;
  int startPos = 0;
  int newlinePos = text.indexOf('\n');
  
  while (startPos < text.length()) {
    String line;
    if (newlinePos == -1) {
      line = text.substring(startPos);
      startPos = text.length();
    } else {
      line = text.substring(startPos, newlinePos);
      startPos = newlinePos + 1;
      newlinePos = text.indexOf('\n', startPos);
    }
    
    int32_t x = mainArea.x + 20;
    int32_t y = mainArea.y + 40 + (currentLine * lineSpacing);
    
    // Use the Czech font instead of FiraSans
    writeln((GFXfont *)&CzechFont, line.c_str(), &x, &y, NULL);
    currentLine++;
  }

  epd_poweroff();
}

// epd_draw_grayscale_image(epd_full_screen(), framebuffer);
// void epd_draw_grayscale_image(Rect_t area, uint8_t *data)
// Parameters:
// Rect_t area
// uint8_t * data
// @brief Draw a picture to a given area. The image area is not cleared and assumed to be white before drawing.
// @param area The display area to draw to. width and height of the area must correspond to the image dimensions in pixels.
// @param data The image data, as a buffer of 4 bit wide brightness values.
// Pixel data is packed (two pixels per byte). A byte cannot wrap over multiple rows, images of uneven width must add a padding nibble per line.


void displayImageOnScreen(String& base64Data) {
  epd_poweron();
  epd_clear_area(mainArea);

  int len = base64Data.length();
  int outLen = len * 3 / 4;
  uint8_t* imageBytes = (uint8_t*)ps_malloc(outLen);
  if (!imageBytes) {
    updateStatusMessage("Chyba: málo paměti");
    return;
  }

  int decodedLen = decode_base64((const unsigned char*)base64Data.c_str(), base64Data.length(), imageBytes);
  if (decodedLen <= 0) {
    updateStatusMessage("Chyba: dekódování base64");
    free(imageBytes);
    return;
  }

  // Display the pre-processed image directly
  // The image is already in the correct format for the display
  epd_draw_grayscale_image(mainArea, imageBytes);
  
  free(imageBytes);
  epd_poweroff();
}

Content pollServer() {
  Content content = { NONE, "", "medium" };  // Default text size
  HTTPClient http;
  String url = String(serverAddress) + pollEndpoint;

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"ip\":\"" + WiFi.localIP().toString() + "\"}";

  int code = http.POST(payload);
  if (code > 0) {
    String response = http.getString();
    Serial.print("Server response: ");
    Serial.println(response);
    
    if (response.indexOf("\"type\":\"text\"") >= 0) content.type = TEXT;
    else if (response.indexOf("\"type\":\"image\"") >= 0) content.type = IMAGE;
    else if (response.indexOf("\"type\":\"clear\"") >= 0) content.type = CLEAR;
    else if (response.indexOf("\"type\":\"processed_image\"") >= 0) content.type = PROCESSED_IMAGE;

    // Extract data
    int start = response.indexOf("\"data\":\"") + 8;
    int end = response.indexOf("\"", start);
    if (start > 7 && end > start) {
      content.data = response.substring(start, end);
      content.data.replace("\\n", "\n");
    }
    
    // Extract textSize with better error handling
    start = response.indexOf("\"textSize\":\"");
    if (start >= 0) {
      start += 12; // Length of "textSize":"
      end = response.indexOf("\"", start);
      if (end > start) {
        content.textSize = response.substring(start, end);
        Serial.print("Extracted text size: ");
        Serial.println(content.textSize);
      }
    }
  } else {
    updateStatusMessage("Chyba připojení k serveru");
  }

  http.end();
  return content;
}

void registerDevice() {
  HTTPClient http;
  http.begin(String(serverAddress) + registerEndpoint);
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"ip\":\"" + WiFi.localIP().toString() + "\"}";
  http.POST(payload);
  http.end();
}

void sendHeartbeat() {
  HTTPClient http;
  http.begin(String(serverAddress) + "/heartbeat");
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"ip\":\"" + WiFi.localIP().toString() + "\"}";
  http.POST(payload);
  http.end();
}

void setup() {
  Serial.begin(115200);
  epd_init();
  epd_fb = (uint8_t*)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT, MALLOC_CAP_SPIRAM);
  if (!epd_fb) {
    Serial.println("Chyba: nedostatek RAM");
    while (true);
  }
  memset(epd_fb, 0xFF, EPD_WIDTH * EPD_HEIGHT);
  framebuffer = epd_fb;
  WiFi.begin(ssid, password);
  updateStatusMessage("Připojuji se k WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  updateStatusMessage("WiFi připojeno");
  registerDevice();
}

void displayProcessedImageOnScreen(String& base64Data) {
  // Report free memory before processing
  Serial.printf("Free heap before processing: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Free PSRAM before processing: %d bytes\n", ESP.getFreePsram());
  
  epd_poweron();
  epd_clear_area(mainArea);
  
  Serial.println("Displaying processed image");
  Serial.print("Base64 data length: ");
  Serial.println(base64Data.length());
  
  int len = base64Data.length();
  int outLen = len * 3 / 4;
  
  uint8_t* imageBytes = (uint8_t*)ps_malloc(outLen);
  if (!imageBytes) {
    updateStatusMessage("Chyba: málo paměti");
    return;
  }

  int decodedLen = decode_base64((const unsigned char*)base64Data.c_str(), base64Data.length(), imageBytes);
  
  if (decodedLen <= 0) {
    updateStatusMessage("Chyba: dekódování base64");
    free(imageBytes);
    return;
  }

  // Display the processed image directly - no need for additional buffer
  // since the server has already formatted it to the exact display dimensions
  epd_draw_grayscale_image(mainArea, imageBytes);
  
  free(imageBytes);
  
  // Report memory after processing
  Serial.printf("Free heap after processing: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Free PSRAM after processing: %d bytes\n", ESP.getFreePsram());
  
  epd_poweroff();
  Serial.println("Image display complete");
}

void loop() {
  static unsigned long lastPoll = 0;
  static unsigned long lastHeartbeat = 0;
  unsigned long now = millis();

  if (now - lastPoll > 10000) {
    Content content = pollServer();
    Serial.println("Polling");
    switch (content.type) {
      case TEXT:
        Serial.print("Displaying text with size: ");
        Serial.println(content.textSize);
        displayTextOnScreen(content.data, content.textSize);
        updateStatusMessage("Zobrazen text");
        break;
      case IMAGE:
        displayImageOnScreen(content.data);
        updateStatusMessage("Zobrazen obrázek");
        break;
      case PROCESSED_IMAGE:
        displayProcessedImageOnScreen(content.data);
        updateStatusMessage("Zobrazen zpracovaný obrázek");
        break;
      case CLEAR:
        epd_poweron(); epd_clear(); epd_poweroff();
        updateStatusMessage("Obrazovka vymazána");
        break;
      default:
        break;
    }
    lastPoll = now;
  }

  if (now - lastHeartbeat > 15000) {
    sendHeartbeat();
    lastHeartbeat = now;
  }

  delay(500);
}

int decode_base64(const unsigned char *input, int length, uint8_t *output) {
  const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int i = 0, j = 0, out_len = 0;
  uint8_t char_array_4[4], char_array_3[3];

  while (length-- && input[i] != '=' && strchr(base64_chars, input[i])) {
    char_array_4[j++] = input[i++];
    if (j == 4) {
      for (j = 0; j < 4; j++)
        char_array_4[j] = strchr(base64_chars, char_array_4[j]) - base64_chars;
      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
      for (j = 0; j < 3; j++)
        output[out_len++] = char_array_3[j];
      j = 0;
    }
  }

  if (j) {
    for (int k = j; k < 4; k++) char_array_4[k] = 0;
    for (int k = 0; k < j; k++)
      char_array_4[k] = strchr(base64_chars, char_array_4[k]) - base64_chars;
    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    for (int k = 0; k < j - 1; k++)
      output[out_len++] = char_array_3[k];
  }

  return out_len;
}
