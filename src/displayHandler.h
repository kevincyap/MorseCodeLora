#pragma once

#include <Arduino.h>

enum class DisplayScreen : uint8_t {
    Idle,
    Composing,
    Sending,
    Receiving,
};

void VextON(void);
void VextOFF(void);
void displaySetup();

// Set the current screen mode.
void displaySetScreen(DisplayScreen screen);

// Idle screen: show status text + last received summary.
void displayIdle(const String &status, const String &lastMsg = "");

// Composing screen: top = raw morse, bottom = decoded preview.
void displayComposing(const String &morseInput, const String &decodedPreview);

// Sending screen: show send status.
void displaySending(const String &status);

// Receiving screen: show incoming decoded text.
void displayReceiving(const String &text, int16_t rssi = 0);

// Dim the display (for power saving).
void displayDim(bool dim);

// Show a simple one-line message (kept for boot/error use).
void displayShowMessage(const String &text);

// Menu screen: show a list with cursor indicator.
void displayMenu(const char* const items[], uint8_t count, uint8_t cursor);

// Page screen: title bar + scrollable content body.
void displayPage(const String &title, const String &content);

