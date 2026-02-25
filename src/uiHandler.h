#pragma once

#include <Arduino.h>
#include "config.h"
#include "inputHandler.h"

// UI states
enum class UIState : uint8_t {
	Main,       // idle / composing
	Menu,       // top-level menu list
	History,    // message history browser
	Address,    // address/target selector
	Settings,   // read-only device info
};

// Menu items shown in the top-level menu
constexpr uint8_t MENU_ITEM_COUNT = 3;
extern const char* const MENU_ITEMS[MENU_ITEM_COUNT];

// Max number of seen devices tracked for address page
constexpr uint8_t MAX_SEEN_DEVICES = 16;

// Initialize UI handler.
void uiSetup();

// Process a button/chord event. Returns true if the event was consumed by the UI
// (i.e., we're in a menu page and main.cpp should NOT handle it).
bool uiHandleInput(ButtonEvent evt, ChordAction chord);

// Current UI state.
UIState uiGetState();

// Notify UI of a received packet (tracks seen devices for address page).
void uiTrackDevice(uint8_t srcID);

// Get current addressing config.
bool uiIsBroadcast();
uint8_t uiGetTargetID();

// History access (shared with main.cpp).
void uiAddHistory(const String &msg);
const String& uiGetHistoryItem(uint8_t idx);
uint8_t uiGetHistoryCount();

// Get last RSSI for settings page.
void uiSetLastRssi(int16_t rssi);
