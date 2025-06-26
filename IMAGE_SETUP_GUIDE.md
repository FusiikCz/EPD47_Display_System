# Průvodce nastavením obrázků pro simple_sd_display.ino

Tento průvodce vysvětluje, jak nastavit obrázky pro ESP32-S3 E-Paper displej s podporou SD karty.

## Instalace Arduino IDE a knihoven

### 1. Instalace Arduino IDE
- Stáhněte a nainstalujte Arduino IDE z oficiálních stránek

### 2. Instalace ESP32 board manager
Nainstalujte Arduino ESP32 verzi 2.0.5 nebo vyšší a nižší než V3.0. Doporučujeme použít verzi 2.0.15.

**Kroky:**
1. Arduino IDE → Preferences → Additional boards manager URLs
2. Přidejte URL: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Tools → Board → Boards Manager
4. Vyhledejte "ESP32" a nainstalujte

### 3. Instalace LilyGo EPD47 knihovny
1. Sketch → Include Library → Manage Libraries
2. Library Search → "LilyGo EPD47"
3. Install → Install ALL

### 4. Nastavení desky
File → Examples → LilyGo EPD47 → Any Examples

**Nastavení Arduino IDE:**

| Nastavení | Hodnota |
|-----------|---------|
| **Board** | **ESP32S3 Dev Module** |
| **Port** | Váš port |
| **USB CDC On Boot** | **Enable** |
| **CPU Frequency** | **240MHZ(WiFi)** |
| **Core Debug Level** | **None** |
| **USB DFU On Boot** | Disable |
| **Erase All Flash Before Sketch Upload** | Disable |
| **Events Run On** | Core1 |
| **Flash Mode** | **QIO 80MHZ** |
| **Flash Size** | **16MB(128Mb)** |
| **Arduino Runs On** | Core1 |
| **USB Firmware MSC On Boot** | Disable |
| **Partition Scheme** | **16M Flash(3M APP/9.9MB FATFS)** |
| **PSRAM** | **OPI PSRAM** |
| **Upload Mode** | UART0/Hardware CDC |
| **Upload Speed** | 921600 |
| **USB Mode** | CDC and JTAG |

**Poznámka:** Tučně zvýrazněné možnosti jsou povinné, ostatní se vybírají podle skutečných podmínek.

### 5. Nahrání kódu
1. Vyberte správný port
2. Klikněte na Upload
3. Počkejte na dokončení kompilace a zápisu

## Hardwarové požadavky

- ESP32-S3 deska s E-Paper displejem
- MicroSD karta (doporučeno ≤32GB)
- SD karta adaptér (pokud je potřeba)
- 2 tlačítka
- Breadboard a propojovací kabely 

## Připojení pinů

### Kompletní schéma zapojení

```
ESP32-S3 Pinout pro simple_sd_display.ino
==========================================

Připojení E-Paper displeje:
┌─────────────────┐
│ E-Paper Displej │
├─────────────────┤
│ CKV    → GPIO38 │
│ STH    → GPIO40 │
│ CKH    → GPIO41 │
│ D0     → GPIO8  │
│ D1     → GPIO1  │
│ D2     → GPIO2  │
│ D3     → GPIO3  │
│ D4     → GPIO4  │
│ D5     → GPIO5  │
│ D6     → GPIO6  │
│ D7     → GPIO7  │
│ VCC    → 3.3V   │
│ GND    → GND    │
└─────────────────┘

Připojení SD karty:
┌─────────────┐
│ MicroSD Karta│
├─────────────┤
│ MISO → GPIO16│
│ MOSI → GPIO15│
│ SCK  → GPIO11│
│ CS   → GPIO42│
│ VCC  → 3.3V  │
│ GND  → GND   │
└─────────────┘

Připojení tlačítek:
┌─────────────┐
│ Další tlačítko│
├─────────────┤
│ Jedna strana → GPIO33│
│ Druhá strana → GND │
└─────────────┘

┌─────────────────┐
│ Předchozí tlačítko│
├─────────────────┤
│ Jedna strana → GPIO32│
│ Druhá strana → GND │
└─────────────────┘
```

### Průvodce připojením krok za krokem

#### 1. Připojení E-Paper displeje
Připojte E-Paper displej k ESP32-S3:

| E-Paper Pin | ESP32-S3 Pin | Popis |
|-------------|--------------|-------|
| CKV         | GPIO38       | Clock Vertical |
| STH         | GPIO40       | Start Horizontal |
| CKH         | GPIO41       | Clock Horizontal |
| D0          | GPIO8        | Datový bit 0 |
| D1          | GPIO1        | Datový bit 1 |
| D2          | GPIO2        | Datový bit 2 |
| D3          | GPIO3        | Datový bit 3 |
| D4          | GPIO4        | Datový bit 4 |
| D5          | GPIO5        | Datový bit 5 |
| D6          | GPIO6        | Datový bit 6 |
| D7          | GPIO7        | Datový bit 7 |
| VCC         | 3.3V         | Napájení | <--baterka
| GND         | GND          | Zem |

#### 2. Připojení SD karty
Připojte SD karta modul k ESP32-S3:

| SD Karta Pin | (slot)


#### 3. Připojení tlačítek
Připojte navigační tlačítka:

| Tlačítko | ESP32-S3 Pin | Připojení |
|----------|--------------|-----------|
| Další    | GPIO33       | Jeden konec na GPIO33, druhý na GND |
| Předchozí| GPIO32       | Jeden konec na GPIO32, druhý na GND |

**Poznámka**: Tlačítka používají interní pull-up rezistory, takže by měla být připojena k GND při stisknutí.

### Tipy pro připojení

1. **Zkontrolujte všechna připojení** před zapnutím napájení
2. **Použijte breadboard** pro snadnější testování a debugování <--hard welded
3. **Zajistěte správné napájení** - SD karty potřebují stabilní 3.3V
4. **Zkontrolujte volná připojení** - zejména na datových pinech
5. **Použijte vhodné délky vodičů** - udržujte připojení co nejkratší

### Testování připojení

Po provedení všech připojení:

1. **Zapněte ESP32-S3**
2. **Otevřete Serial Monitor** (115200 baud)
3. **Zkontrolujte inicializační zprávy**:
   - "Hello, ESP32!"
   - "After epd_init()"
   - "After framebuffer allocation"
   - "SD karta připravena." (pokud SD karta funguje)

4. **Pokud vidíte chyby**, zkontrolujte konkrétní komponentu zmíněnou v chybové zprávě

## Nastavení SD karty

### 1. Formátování SD karty
- Formátujte SD kartu jako **FAT32**
- Použijte kartu ≤32GB pro nejlepší kompatibilitu
- Ujistěte se, že je karta správně formátována (ne jen smazána)

### 2. Vytvoření složky s obrázky
- Vytvořte složku s názvem `images` v kořenovém adresáři SD karty
- Struktura složek by měla být:
  ```
  Kořen SD karty/
  └── images/
      ├── image1.raw
      ├── image2.raw
      └── ...
  ```

## Požadavky na formát obrázků

### Podporované formáty
- **RAW formát** (doporučeno)  <-- aktivně používáme 
- **BMP formát** (základní podpora)

### Specifikace obrázků
- **Rozlišení**: 960×540 pixelů
- **Barevná hloubka**: 4-bit grayscale (16 úrovní)
- **Velikost souboru**: Přesně 259,200 bajtů (960×540÷2)

### Konverze obrázků

#### Možnost 1: Použití poskytovaného konvertoru
1. Spusťte `image_converter.py`    "python image_converter.py --gui"
2. Vložte váš obrázek (JPG, PNG, atd.)
3. Výstup bude .raw soubor připravený pro displej

#### Možnost 2: Ruční konverze
1. Změňte velikost obrázku na 960×540 pixelů
2. Převeďte na 4-bit grayscale
3. Uložte jako surová binární data
4. Ujistěte se, že velikost souboru je přesně 259,200 bajtů

## Ovládání tlačítek

Zařízení má dvě tlačítka pro navigaci:
- **Další tlačítko** (GPIO33): Přejít na další obrázek
- **Předchozí tlačítko** (GPIO32): Přejít na předchozí obrázek
- **Obě tlačítka**: Obnovit seznam obrázků ze SD karty

### Chování tlačítek
- Stisknutí tlačítek probudí zařízení z hlubokého spánku
- Obrázky se cyklují v smyčce (poslední obrázek → první obrázek)
- Současné stisknutí obou tlačítek obnoví seznam obrázků

## Návod k použití

1. **Zapnutí**: Zařízení zobrazí první nalezený obrázek
2. **Navigace**: Použijte tlačítka pro procházení obrázků
3. **Obnovení**: Stiskněte obě tlačítka pro znovu načtení obrázků ze SD karty
4. **Spánek**: Zařízení automaticky přejde do hlubokého spánku mezi stisknutími tlačítek

## Řešení problémů

### Žádné obrázky se nezobrazují
- Zkontrolujte formát SD karty (musí být FAT32)
- Ověřte, že složka `images` existuje v kořenovém adresáři
- Ujistěte se, že obrázky jsou ve správném formátu a velikosti
- Zkontrolujte zapojení SD karty

### SD karta není detekována
- Ověřte, že všechny piny SD karty jsou správně připojeny
- Zkuste jinou SD kartu
- Zkontrolujte napájení (SD karty potřebují stabilní 3.3V)
- Ujistěte se, že je karta správně vložena

### Problémy s displejem
- Zkontrolujte připojení E-Paper displeje
- Ověřte formát a velikost obrázků
- Ujistěte se, že je na vaší desce dostupná PSRAM

### Tlačítka nefungují
- Zkontrolujte zapojení tlačítek na GPIO32 a GPIO33
- Ověřte, že tlačítka jsou připojena k zemi při stisknutí
- Ujistěte se, že tlačítka nekonfliktují s jiným hardwarem

## Příklad struktury souborů

```
SD Karta/
├── images/
│   ├── photo1.raw
│   ├── photo2.raw
│   ├── photo3.raw
│   └── photo4.raw
└── (jiné soubory)
```

## Technické poznámky

- Zařízení používá hluboký spánek pro úsporu energie
- Obrázky se načítají ze SD karty podle potřeby
- Displej se kompletně obnovuje pro každý obrázek
- Je implementováno potlačení kmitání tlačítek (100ms zpoždění)
- Maximálně 10 obrázků podporováno (konfigurovatelné v kódu)

## Konfigurace kódu

Klíčová nastavení v `simple_sd_display.ino`:
- `SD_CS_PIN 42`: Pin pro chip select SD karty
- `IMAGE_FOLDER "/images"`: Složka obsahující obrázky
- `MAX_IMAGE_COUNT 10`: Maximální počet obrázků
- `PIN_NEXT_BTN 33`: Pin pro tlačítko "další"
- `PIN_PREV_BTN 32`: Pin pro tlačítko "předchozí"