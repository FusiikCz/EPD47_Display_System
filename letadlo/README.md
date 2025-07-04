# letadlo: E-Paper Image Viewer for LilyGO TTGO T5-4.7" ESP32

This project is a low-power image viewer for the [LilyGO TTGO T5-4.7" E-Paper ESP32 WiFi Module](https://www.laskakit.cz/en/lilygo-ttgo-t5-4-7--e-paper-esp32-wifi-modul/). It allows you to display and navigate grayscale images from an SD card using two buttons, with ultra-low standby power thanks to deep sleep support.

## Features
- Supports 4.7" 960x540px E-Paper display (16 shades of gray)
- Reads images from microSD card (FAT32, ≤32GB recommended)
- Two-button navigation (Next/Previous)
- Ultra-low power: <0.017mW standby using deep sleep
- Remembers last image using RTC memory
- Simple image conversion utility included
- Czech and English documentation

## Quick Start
1. **Prepare your hardware:**
   - LilyGO TTGO T5-4.7" E-Paper ESP32 board
   - microSD card (FAT32, ≤32GB)
   - Two push buttons
   - Wires, breadboard (optional)
2. **Flash the firmware:**
   - Open `simple_sd_display.ino` in Arduino IDE
   - Select board: `ESP32S3 Dev Module`
   - Install required libraries
   - Upload the code
3. **Prepare your SD card:**
   - Format as FAT32
   - Create a folder `images` in the root
   - Use `image_converter.py` to convert your images to `.raw` (4-bit grayscale, 960x540)
   - Copy `.raw` files to `images/`
4. **Wire the buttons:**
   - NEXT: GPIO47 → GND (use INPUT_PULLUP)
   - PREV: GPIO17 → GND (use INPUT_PULLUP)
5. **Power the board:**
   - USB-C, 18650 battery, or 3.3V supply

## How It Works
- On boot or button press, the device wakes up, shows the next/previous image, and returns to deep sleep for ultra-low power.
- The E-Paper display retains the image with zero power between wakeups.
- The current image index is stored in RTC memory, so navigation is seamless.

## Wiring Summary
- **E-Paper display:** Connected as per LilyGO T5-4.7" documentation
- **SD card:** SCK=11, MISO=16, MOSI=15, CS=42
- **Buttons:** GPIO47 (Next), GPIO17 (Prev), both to GND

## Power Consumption
- Standby: <0.017mW (deep sleep, only RTC domain active)
- Active: ~26mW during image refresh

---


**Battery:** 3.7V, 1500mAh (5.55Wh)

| Mode                | Power      | Current      | Battery Life (1500mAh) |
|---------------------|------------|--------------|------------------------|
| Standby (deep sleep)| 0.017 mW   | 4.6 μA       | ~37 years              |
| Active (refresh)    | 26 mW      | 7.03 mA      | ~385,000 presses       |
| 20 presses/day      | 0.696 mWh/day | —         | ~22 years              |

#### Calculation Details
- **Standby (deep sleep):**
  - Current: I = P/V = 0.017mW / 3.7V ≈ 0.0046mA (4.6μA)
  - Battery life: 1500mAh / 0.0046mA ≈ 326,087 hours ≈ 37 years
- **Active (image refresh):**
  - Current: I = 26mW / 3.7V ≈ 7.03mA
  - Each press: ~2s at 26mW → 0.0144mWh per press
  - Battery life: 5.55Wh / 0.0000144Wh ≈ 385,000 presses
- **Mixed use (20 presses/day):**
  - Active: 20 × 0.0144mWh = 0.288mWh/day
  - Standby: 0.017mW × 24h = 0.408mWh/day
  - Total: 0.696mWh/day → 5,550mWh / 0.696mWh ≈ 7,978 days ≈ 22 years

*Real-world battery life will be lower due to self-discharge, temperature, and battery aging, but this demonstrates the ultra-low power design.*

---

## Documentation
- [IMAGE_SETUP_GUIDE.md](./IMAGE_SETUP_GUIDE.md): Full setup, wiring, and troubleshooting
- [image_converter.py](./image_converter.py): Image conversion utility
- [LaskaKit product page](https://www.laskakit.cz/en/lilygo-ttgo-t5-4-7--e-paper-esp32-wifi-modul/)


---