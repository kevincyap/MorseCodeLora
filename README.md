# MorseCodeLora

A battery-optimized, bidirectional morse code communicator built on the **Heltec WiFi LoRa 32 V4** (ESP32-S3 + SX1262). Users compose messages with two buttons (dot and dash), transmit via LoRa, and receive messages as text on the OLED display and vibration patterns through a motor.

## Hardware

| Component | Details |
|---|---|
| Board | [Heltec WiFi LoRa 32 V4](https://heltec.org/project/wifi-lora-32-v4/) — ESP32-S3, SX1262, SSD1306 128×64 OLED |
| Dot button | GPIO 6, active LOW with internal pull-up |
| Dash button | GPIO 7, active LOW with internal pull-up |
| Send button | GPIO 5, active LOW with internal pull-up |
| Vibration motor | GPIO 19, driven via MOSFET/transistor |

Pin assignments are in [`src/config.h`](src/config.h).

## Features

- **Two-button morse input** — dot and dash buttons with live decode preview on the OLED
- **Bidirectional LoRa** — addressed or broadcast messaging with ACK support
- **Vibration feedback** — notification buzz followed by morse pattern replay on receive
- **Button chording** — simultaneous button combos for backspace, history, addressing, and force sleep
- **Range-optimized radio** — SF12, CR 4/8, 22 dBm TX power for maximum range
- **Power management** — WiFi/BT disabled, display dim after 30s, deep sleep after 2min, wake on button press
- **Device identity** — random 1-byte ID assigned on first boot, persisted in NVS

## Button Controls

### Single Buttons
| Button | Action |
|---|---|
| Dot | Append `.` to current morse character |
| Dash | Append `-` to current morse character |
| Send | Transmit the composed message |

### Chords (simultaneous press)
| Chord | Action |
|---|---|
| Dot + Dash | Scroll through message history |
| Dot + Send | Backspace (delete last symbol/character) |
| Dash + Send | Toggle broadcast / addressed mode |
| Dot + Dash + Send | Force deep sleep |

Chord mappings are configurable in [`src/chordMappings.h`](src/chordMappings.h).

## Composing a Message

1. Press **Dot** and **Dash** buttons to enter morse symbols
2. After 500ms of inactivity, the current symbol sequence is auto-decoded to a character
3. The OLED shows the raw morse (top) and decoded text preview (bottom)
4. Press **Send** to transmit

## Architecture

```
src/
├── config.h             Central pin definitions, LoRa params, timing constants
├── chordMappings.h      Configurable chord-to-action mapping table
├── main.cpp             Application loop: compose → send → receive flow
├── inputHandler.*       Debounced 3-button input with chord detection
├── morseCodec.*         ITU morse encode/decode lookup table
├── packetProtocol.*     Packet framing: flags, addressing, sequence numbers
├── radioHandler.*       SX1262 radio driver (RadioLib wrapper)
├── cryptoHandler.*      AES-128-CTR encryption/decryption via mbedtls
├── displayHandler.*     SSD1306 OLED multi-screen UI
├── vibrationHandler.*   Non-blocking vibration motor state machine
├── deviceIdentity.*     NVS-backed device ID
└── powerHandler.*       WiFi/BT disable, dim, deep sleep with EXT1 wake
```

## Packet Format

```
[flags:1] [srcID:1] [dstID:1] [seqNum:1] [text\0morse\0]
```

- **flags** — bit 0: ACK request, bit 1: is ACK, bit 2: broadcast
- **srcID / dstID** — 1-byte device IDs (0xFF = broadcast)
- **seqNum** — wrapping sequence number for ACK correlation
- **payload** — AES-128-CTR encrypted: `[nonce:4][ciphertext:N]` (ACK packets have no payload)

## LoRa Parameters

| Parameter | Value | Rationale |
|---|---|---|
| Frequency | 915 MHz | ISM band |
| Spreading Factor | 12 | Maximum range |
| Bandwidth | 125 kHz | Good range/robustness balance |
| Coding Rate | 4/8 | Maximum forward error correction |
| TX Power | 22 dBm | SX1262 maximum |
| Preamble | 12 symbols | Improved detection at distance |

## Building

Requires [PlatformIO](https://platformio.org/).

```bash
# Compile
pio run

# Upload to board
pio run -t upload

# Serial monitor (115200 baud)
pio device monitor
```

## Device Setup

After flashing, open the serial monitor and configure each device:

```
name:Alpha          # Set device name (max 16 chars, persists in NVS)
id:2A               # Set device ID as hex 01-FE (persists in NVS)
```

These persist through power cycles and reflashes. If no name is set, it defaults to "Device XX" where XX is the hex ID.

## Encryption

Messages are encrypted with AES-128-CTR using a shared key compiled into the firmware. Only devices built from the same source can decode each other's messages. The packet header (addressing, flags) remains plaintext for routing.

To create a private network, change `CRYPTO_KEY` in [`src/config.h`](src/config.h) and rebuild all devices:

```cpp
constexpr uint8_t CRYPTO_KEY[16] = {
    0x4D, 0x79, 0x53, 0x65, 0x63, 0x72, 0x65, 0x74,  // change these
    0x4B, 0x65, 0x79, 0x48, 0x65, 0x72, 0x65, 0x21    // 16 bytes total
};
```

## Power States

```
DEEP_SLEEP ──(button press)──▶ ACTIVE
ACTIVE ──(30s idle)──▶ DISPLAY_DIM
DISPLAY_DIM ──(2min idle)──▶ DEEP_SLEEP
Any state ──(button/RX)──▶ ACTIVE
```

In deep sleep, WiFi and BT are disabled, the OLED is powered off (Vext gated), and the device wakes on any button press via EXT1.

## License

This project is unlicensed. See the repository for details.
