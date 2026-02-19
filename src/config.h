#pragma once

// =============================================================================
// MorseCodeLora — Central Configuration
// =============================================================================

#include <Arduino.h>

// ---- GPIO Pin Assignments ---------------------------------------------------

// Buttons (active LOW, internal pull-up)
constexpr uint8_t PIN_BTN_DOT  = 47;
constexpr uint8_t PIN_BTN_DASH = 48;
constexpr uint8_t PIN_BTN_SEND = 26;

// Vibration motor (PWM-capable output)
constexpr uint8_t PIN_VIBRATION = 46;

// ---- Button Bitmasks --------------------------------------------------------
constexpr uint8_t BTN_MASK_DOT  = 0x01;  // bit 0
constexpr uint8_t BTN_MASK_DASH = 0x02;  // bit 1
constexpr uint8_t BTN_MASK_SEND = 0x04;  // bit 2

// ---- Input Timing (ms) -----------------------------------------------------
constexpr unsigned long DEBOUNCE_MS       = 30;
constexpr unsigned long CHORD_WINDOW_MS   = 50;
constexpr unsigned long CHAR_TIMEOUT_MS   = 500;  // pause after last dot/dash → decode char

// ---- LoRa Parameters (range-optimized) --------------------------------------
constexpr float    LORA_FREQUENCY_MHZ   = 915.0F;
constexpr float    LORA_BANDWIDTH_KHZ   = 125.0F;
constexpr uint8_t  LORA_SPREADING_FACTOR = 12;
constexpr uint8_t  LORA_CODING_RATE     = 8;      // denominator: 4/8
constexpr int8_t   LORA_TX_POWER_DBM    = 22;
constexpr uint16_t LORA_PREAMBLE_LENGTH = 12;
constexpr uint8_t  LORA_SYNC_WORD       = 0x12;   // private network

// ---- Packet Protocol --------------------------------------------------------
constexpr uint8_t PACKET_FLAG_ACK_REQ   = 0x01;   // bit 0: sender wants ACK
constexpr uint8_t PACKET_FLAG_IS_ACK    = 0x02;   // bit 1: this is an ACK
constexpr uint8_t PACKET_FLAG_BROADCAST = 0x04;   // bit 2: broadcast message
constexpr uint8_t BROADCAST_ADDR        = 0xFF;
constexpr unsigned long ACK_TIMEOUT_MS  = 3000;

// ---- Vibration Timing (ms) --------------------------------------------------
constexpr unsigned long VIB_NOTIFY_MS      = 200;  // initial notification buzz
constexpr unsigned long VIB_DOT_MS         = 100;
constexpr unsigned long VIB_DASH_MS        = 300;
constexpr unsigned long VIB_SYMBOL_GAP_MS  = 100;  // gap between dot/dash
constexpr unsigned long VIB_CHAR_GAP_MS    = 200;  // gap between characters

// ---- Power Management -------------------------------------------------------
constexpr unsigned long DISPLAY_DIM_MS  = 30000;   // 30s idle → dim
constexpr unsigned long DEEP_SLEEP_MS   = 120000;  // 2min idle → deep sleep

// ---- Message History --------------------------------------------------------
constexpr uint8_t MESSAGE_HISTORY_SIZE = 10;
