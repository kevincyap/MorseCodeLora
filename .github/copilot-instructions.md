# Copilot Instructions for MorseCodeLora

## Project Overview
This is a PlatformIO Arduino project targeting the **Heltec WiFi LoRa 32 V4** (ESP32-S3 + SX1262 radio + SSD1306 OLED). It implements a bidirectional morse code communicator over LoRa with two-button input, vibration motor feedback, and aggressive power management.

## Build & Upload
- Build: `pio run` (or full path `~/.platformio/penv/Scripts/pio.exe run` on Windows)
- Upload: `pio run -t upload`
- Clean: `pio run -t clean`
- Monitor: `pio device monitor` (115200 baud)
- PlatformIO may not be on PATH; use the full path at `~/.platformio/penv/Scripts/pio.exe` if needed.

## Code Conventions
- **C++11 compatible** — the ESP32 Arduino toolchain (GCC 8.4) does not support C++14+. Do not use default member initializers in structs that need aggregate initialization.
- **No PROGMEM** — ESP32 has unified flash/RAM address space; PROGMEM is a no-op and complicates pointer access. Use plain `const` arrays.
- **ISR safety** — use `portMUX_TYPE` / `portENTER_CRITICAL` for variables shared between ISR and main loop on the dual-core ESP32-S3. `volatile` alone is insufficient.
- **String use** — Arduino `String` is used for convenience but be mindful of heap fragmentation. For fixed-size buffers, prefer `char[]` with `reserve()`.
- **Header-only configs** — `config.h` holds all pin/timing/radio constants. `chordMappings.h` holds the chord mapping table. Both are `constexpr` for zero-cost.

## Architecture
All source files live in `src/`. There are no private libraries in `lib/`.

| Module | Purpose |
|---|---|
| `config.h` | Pin definitions, LoRa params, timing constants |
| `chordMappings.h` | Chord-to-action mapping table (user-editable) |
| `main.cpp` | Application state machine and loop |
| `inputHandler` | 3-button debounce + chord detection (50ms window, peak mask) |
| `morseCodec` | ITU morse encode/decode (A-Z, 0-9, punctuation) |
| `packetProtocol` | Serialize/deserialize framed packets with addressing |
| `radioHandler` | SX1262 via RadioLib — init, TX, RX, ACK, duty cycle |
| `cryptoHandler` | AES-128-CTR encrypt/decrypt using ESP32 hardware mbedtls |
| `displayHandler` | SSD1306 OLED — idle, composing, sending, receiving screens |
| `vibrationHandler` | Non-blocking state machine for buzz + morse pattern replay |
| `deviceIdentity` | 1-byte device ID in ESP32 NVS (Preferences API) |
| `powerHandler` | WiFi/BT disable, display dim, deep sleep with EXT1 wake |

## Key Design Decisions
- LoRa is range-optimized: SF12, CR 4/8, 22 dBm, 125 kHz BW. Slow (~250 bps) but fine for short text.
- Packets carry both decoded ASCII text and raw morse string (for vibration replay).
- Chord detection uses a rolling 50ms window with peak mask accumulation — the window resets on each new button press.
- Deep sleep uses EXT1 wake on button GPIOs. Always validate `esp_sleep_enable_ext1_wakeup()` return before sleeping.
- The custom board variant is in `variants/heltec_V4/` with `pins_arduino.h` defining all Heltec-specific pin mappings.

## Hardware Pins (from config.h)
- Buttons: GPIO 6 (dot), 7 (dash), 5 (send) — active LOW, internal pull-up
- Vibration motor: GPIO 19
- OLED: SDA=17, SCL=18, RST=21 (Vext-gated on GPIO 36)
- Radio SPI: SS=8, SCK=9, MOSI=10, MISO=11, RST=12, BUSY=13, DIO0=14
