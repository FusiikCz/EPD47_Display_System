#include <Arduino.h>
#include <SD.h>
#include "epd_driver.h"
#include "utilities.h"
#include "firasans.h"

// Tlačítka
#define PIN_NEXT_BTN 47
#define PIN_PREV_BTN 17

#define BUTTON_DEBOUNCE_MS 100

// Button state tracking
bool lastNextButtonState = HIGH;
bool lastPrevButtonState = HIGH;
unsigned long lastNextButtonTime = 0;
unsigned long lastPrevButtonTime = 0;

// Volba zdroje obrázků
#define USE_EMBEDDED_IMAGES false  // true = obrázky v kódu, false = SD karta
#define IMAGE_FOLDER "/images"
#define MAX_IMAGE_COUNT 10

// Rozlišení displeje
#define EPD_WIDTH 960
#define EPD_HEIGHT 540

#define SD_CS_PIN 42

SPIClass spi(HSPI);

uint8_t *framebuffer;

String imageFiles[MAX_IMAGE_COUNT];
int imageCount = 0;
int currentImageIndex = 0;

#if USE_EMBEDDED_IMAGES
const uint8_t image1[] PROGMEM = {
  // testovací obsah
};

const uint8_t image2[] PROGMEM = {
  // testovací obsah
};

const uint8_t* embeddedImages[] = {image1, image2};
const int embeddedImageCount = sizeof(embeddedImages) / sizeof(embeddedImages[0]);
#endif

void handleWakeup() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1 && imageCount > 0) {
        uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();

        // Both buttons pressed: re-scan SD card for new images
        if ((wakeup_pin_mask & (1ULL << PIN_NEXT_BTN)) && (wakeup_pin_mask & (1ULL << PIN_PREV_BTN))) {
            Serial.println("Obě tlačítka stisknuta: obnovuji seznam obrázků ze SD karty...");
            loadImageList();
            currentImageIndex = 0;
            delay(BUTTON_DEBOUNCE_MS);
        } else if (wakeup_pin_mask & (1ULL << PIN_NEXT_BTN)) {
            currentImageIndex = (currentImageIndex + 1) % imageCount;
            Serial.println("Next button pressed");
            delay(BUTTON_DEBOUNCE_MS);
        } else if (wakeup_pin_mask & (1ULL << PIN_PREV_BTN)) {
            currentImageIndex = (currentImageIndex - 1 + imageCount) % imageCount;
            Serial.println("Previous button pressed");
            delay(BUTTON_DEBOUNCE_MS);
        }
    }
}

void loadImageList() {
    File root = SD.open(IMAGE_FOLDER);
    if (!root || !root.isDirectory()) {
        Serial.println("Chyba: složka /images neexistuje nebo není adresář.");
        imageCount = 0;
        return;
    }

    imageCount = 0;
    File file = root.openNextFile();
    while (file && imageCount < MAX_IMAGE_COUNT) {
        String filename = String(file.name());
        if (filename.endsWith(".bmp") || filename.endsWith(".raw")) {
            imageFiles[imageCount++] = filename;
            Serial.println("Nalezen obrázek: " + filename);
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();
    Serial.printf("Obnoveno, nalezeno obrázků: %d\n", imageCount);
}

// --- SD Card Error Display Function ---
void displaySdErrorMessage(const char* message) {
    epd_poweron();
    epd_clear();
    int32_t x = EPD_WIDTH / 2 - 200;
    int32_t y = EPD_HEIGHT / 2;
    writeln((GFXfont *)&FiraSans, message, &x, &y, NULL);
    epd_poweroff();
}

// --- SD Card Initialization with Feedback ---
bool initializeSDCard(bool showOnDisplay = false) {
    int sdTries = 0;
    bool sdOk = false;
    spi.begin(11, 16, 15, 42); // SCK, MISO, MOSI, CS
    while (sdTries < 3 && !sdOk) {
        if (SD.begin(SD_CS_PIN, spi)) {
            sdOk = true;
            break;
        } else {
            Serial.printf("Chyba: SD karta nenalezena (pokus %d).\n", sdTries + 1);
            Serial.print("Zkontrolujte zapojení: MOSI, MISO, SCK, CS (aktuální CS_PIN=");
            Serial.print(SD_CS_PIN);
            Serial.println(")");
            if (showOnDisplay) {
                displaySdErrorMessage("Chyba: SD karta\nnenalezena!\nZkontrolujte zapojení.");
            }
            delay(1000);
        }
        sdTries++;
    }
    if (!sdOk && showOnDisplay) {
        displaySdErrorMessage("Chyba: SD karta\nnenalezena po 3 pokusech!\nZkontrolujte napájení, formátování (FAT32), velikost karty (<32GB) a zapojení.");
    }
    return sdOk;
}

// --- Check SD Card Before Each Image Load ---
bool ensureSDCardReady() {
    if (!SD.cardType() || SD.cardType() == CARD_NONE) {
        Serial.println("SD karta není připravena, pokouším se znovu inicializovat...");
        if (!initializeSDCard(true)) {
            Serial.println("SD karta stále není dostupná.");
            return false;
        }
        loadImageList();
    }
    return true;
}

bool loadImageToFramebuffer(const char *filename) {
#if USE_EMBEDDED_IMAGES
    if (currentImageIndex < embeddedImageCount) {
        memcpy_P(framebuffer, embeddedImages[currentImageIndex], EPD_WIDTH * EPD_HEIGHT / 2);
        return true;
    }
    return false;
#else
    if (!ensureSDCardReady()) {
        displaySdErrorMessage("Chyba: SD karta\nnenalezena nebo selhala!");
        return false;
    }
    String path = String(IMAGE_FOLDER) + "/" + filename;
    File imgFile = SD.open(path.c_str());
    if (!imgFile) {
        Serial.println("Chyba při otevírání: " + path);
        displaySdErrorMessage(("Chyba při otevírání souboru:\n" + path).c_str());
        return false;
    }
    size_t expectedSize = EPD_WIDTH * EPD_HEIGHT / 2;
    if (imgFile.size() != expectedSize) {
        Serial.printf("Chybná velikost (%s): %d B (očekáváno %d B)\n",
                      filename, imgFile.size(), expectedSize);
        imgFile.close();
        displaySdErrorMessage(("Chybná velikost souboru:\n" + String(filename)).c_str());
        return false;
    }
    size_t bytesRead = imgFile.read(framebuffer, expectedSize);
    imgFile.close();
    if (bytesRead != expectedSize) {
        Serial.printf("Nepodařilo se načíst celý obrázek: %s\n", filename);
        displaySdErrorMessage(("Chyba při čtení souboru:\n" + String(filename)).c_str());
        return false;
    }
    return true;
#endif
}

void displayCurrentImage() {
    epd_poweron();
    epd_clear(); // Clear display to prevent overlapping

#if USE_EMBEDDED_IMAGES
    if (loadImageToFramebuffer(nullptr)) {
#else
    if (loadImageToFramebuffer(imageFiles[currentImageIndex].c_str())) {
#endif
        Rect_t area = {
            .x = 0,
            .y = 0,
            .width = EPD_WIDTH,
            .height = EPD_HEIGHT
        };
        epd_draw_grayscale_image(area, framebuffer);
        Serial.printf("Displaying image index: %d, filename: %s\n", currentImageIndex, imageFiles[currentImageIndex].c_str());
    }

    epd_poweroff();
    delay(100); // Small delay to let display settle
}

void displayNoImagesMessage() {
    epd_poweron();
    epd_clear();

    const char* message = "Žádné obrázky nenalezeny";
    int32_t x = EPD_WIDTH / 2 - 200;
    int32_t y = EPD_HEIGHT / 2;

    writeln((GFXfont *)&FiraSans, message, &x, &y, NULL);

    epd_poweroff();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Hello, ESP32!");
//dočasné
    Serial.printf("Next button pin: GPIO%d\n", PIN_NEXT_BTN);
    Serial.printf("Prev button pin: GPIO%d\n", PIN_PREV_BTN);

    // handleWakeup(); // Removed - no deep sleep

    Serial.println("Inicializace EPD...");
    epd_init();
    Serial.println("After epd_init()");
    
    // Clean display on bootup
    epd_poweron();
    epd_clear();
    epd_poweroff();
    Serial.println("Display cleared on bootup");

    if (!psramFound()) {
        Serial.println("Chyba: PSRAM nenalezena!");
        displaySdErrorMessage("Chyba: PSRAM\nnenalezena!");
        while (1);
    }

    framebuffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
    if (!framebuffer) {
        Serial.println("Chyba: Alokace framebufferu selhala.");
        displaySdErrorMessage("Chyba: Alokace\nframebufferu selhala!");
        while (1);
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    Serial.println("After framebuffer allocation");

    pinMode(PIN_NEXT_BTN, INPUT_PULLUP);
    pinMode(PIN_PREV_BTN, INPUT_PULLUP);

#if USE_EMBEDDED_IMAGES
    imageCount = embeddedImageCount;
    Serial.printf("Používám vestavěné obrázky: %d\n", imageCount);
#else
    Serial.println("Before SD card init");
    Serial.println("Inicializace SD karty...");
    if (!initializeSDCard(true)) {
        imageCount = 0;
    } else {
        Serial.println("SD karta připravena.");
        uint8_t cardType = SD.cardType();
        if (cardType == CARD_NONE) Serial.println("Typ karty: žádná");
        else if (cardType == CARD_MMC) Serial.println("Typ karty: MMC");
        else if (cardType == CARD_SD) Serial.println("Typ karty: SDSC");
        else if (cardType == CARD_SDHC) Serial.println("Typ karty: SDHC/SDXC");
        else Serial.println("Typ karty: neznámý");
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("Velikost karty: %llu MB\n", cardSize);
        loadImageList();
        Serial.printf("Nalezeno obrázků: %d\n", imageCount);
    }
    Serial.println("After SD card init");
#endif
}

void loop() {
    if (imageCount > 0) {
        // Show current image
        displayCurrentImage();
        Serial.printf("Displayed image %d: %s\n", currentImageIndex, imageFiles[currentImageIndex].c_str());
        
        // Wait for button press
        while (true) {
            // Check for button presses
            bool currentNextButtonState = digitalRead(PIN_NEXT_BTN);
            bool currentPrevButtonState = digitalRead(PIN_PREV_BTN);
            unsigned long currentTime = millis();
            
            // Next button pressed (LOW = pressed due to INPUT_PULLUP)
            if (currentNextButtonState == LOW && lastNextButtonState == HIGH && 
                (currentTime - lastNextButtonTime) > BUTTON_DEBOUNCE_MS) {
                lastNextButtonTime = currentTime;
                currentImageIndex = (currentImageIndex + 1) % imageCount;
                Serial.printf("Next button pressed - switching to image %d: %s\n", currentImageIndex, imageFiles[currentImageIndex].c_str());
                break; // Exit the wait loop to show new image
            }
            
            // Previous button pressed
            if (currentPrevButtonState == LOW && lastPrevButtonState == HIGH && 
                (currentTime - lastPrevButtonTime) > BUTTON_DEBOUNCE_MS) {
                lastPrevButtonTime = currentTime;
                currentImageIndex = (currentImageIndex - 1 + imageCount) % imageCount;
                Serial.printf("Prev button pressed - switching to image %d: %s\n", currentImageIndex, imageFiles[currentImageIndex].c_str());
                break; // Exit the wait loop to show new image
            }
            
            // Update button states
            lastNextButtonState = currentNextButtonState;
            lastPrevButtonState = currentPrevButtonState;
            
            delay(10); // Short delay for responsive button detection
        }
    } else {
        displayNoImagesMessage();
        Serial.println("No images found, displayed message.");
        delay(10000);
    }
}
