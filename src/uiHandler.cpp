#include "uiHandler.h"
#include "displayHandler.h"
#include "deviceIdentity.h"
#include "powerHandler.h"

const char* const MENU_ITEMS[MENU_ITEM_COUNT] = {
	"History",
	"Address",
	"Settings",
};

namespace {
	UIState state = UIState::Main;
	uint8_t menuCursor = 0;

	// History
	String historyBuf[MESSAGE_HISTORY_SIZE];
	uint8_t historyCount = 0;
	uint8_t historyCursor = 0;

	// Address
	bool broadcastMode = true;
	uint8_t targetID = BROADCAST_ADDR;
	uint8_t seenDevices[MAX_SEEN_DEVICES];
	uint8_t seenCount = 0;
	uint8_t addrCursor = 0;  // 0 = broadcast toggle, 1..N = seen devices

	// Settings
	int16_t lastRssi = 0;

	// ---- Drawing helpers ------------------------------------------------

	void drawMenu() {
		displayMenu(MENU_ITEMS, MENU_ITEM_COUNT, menuCursor);
	}

	void drawHistory() {
		if (historyCount == 0) {
			displayPage("History", "No messages");
			return;
		}
		String label = String(historyCursor + 1) + "/" + String(historyCount);
		displayPage("History " + label, historyBuf[historyCursor]);
	}

	void drawAddress() {
		// Build items list: first item is broadcast toggle, rest are seen devices
		String lines;
		uint8_t totalItems = 1 + seenCount;

		for (uint8_t i = 0; i < totalItems; ++i) {
			if (i > 0) lines += '\n';
			lines += (i == addrCursor) ? "> " : "  ";
			if (i == 0) {
				lines += broadcastMode ? "[x] Broadcast" : "[ ] Broadcast";
			} else {
				uint8_t devID = seenDevices[i - 1];
				lines += "ID: 0x" + String(devID, HEX);
				if (!broadcastMode && devID == targetID) {
					lines += " *";
				}
			}
		}
		displayPage("Address", lines);
	}

	void drawSettings() {
		String info;
		info += "Name: " + deviceGetName() + "\n";
		info += "ID:   0x" + String(deviceGetID(), HEX) + "\n";
		info += "RSSI: " + String(lastRssi) + " dBm\n";
		info += "Mode: " + String(broadcastMode ? "Broadcast" : "Addressed");
		displayPage("Settings", info);
	}

	void enterMenu() {
		state = UIState::Menu;
		menuCursor = 0;
		drawMenu();
	}

	void enterPage(uint8_t menuIdx) {
		switch (menuIdx) {
			case 0:
				state = UIState::History;
				historyCursor = (historyCount > 0) ? historyCount - 1 : 0;
				drawHistory();
				break;
			case 1:
				state = UIState::Address;
				addrCursor = 0;
				drawAddress();
				break;
			case 2:
				state = UIState::Settings;
				drawSettings();
				break;
		}
	}

	void exitToMenu() {
		state = UIState::Menu;
		drawMenu();
	}

	void exitToMain() {
		state = UIState::Main;
	}

	// ---- Input handlers per state ----------------------------------------

	bool handleMenu(ButtonEvent evt, ChordAction chord) {
		if (evt == ButtonEvent::DotPress) {
			if (menuCursor > 0) menuCursor--;
			drawMenu();
			return true;
		}
		if (evt == ButtonEvent::DashPress) {
			if (menuCursor < MENU_ITEM_COUNT - 1) menuCursor++;
			drawMenu();
			return true;
		}
		if (evt == ButtonEvent::SendPress) {
			exitToMain();
			return true;
		}
		if (evt == ButtonEvent::Chord && chord == ChordAction::Select) {
			enterPage(menuCursor);
			return true;
		}
		if (evt == ButtonEvent::Chord && chord == ChordAction::ForceSleep) {
			powerDeepSleep();
			return true;
		}
		return true;  // consume all events while in menu
	}

	bool handleHistory(ButtonEvent evt, ChordAction /*chord*/) {
		if (historyCount == 0) {
			if (evt == ButtonEvent::SendPress) {
				exitToMenu();
			}
			return true;
		}
		if (evt == ButtonEvent::DotPress) {
			if (historyCursor > 0) historyCursor--;
			drawHistory();
			return true;
		}
		if (evt == ButtonEvent::DashPress) {
			if (historyCursor < historyCount - 1) historyCursor++;
			drawHistory();
			return true;
		}
		if (evt == ButtonEvent::SendPress) {
			exitToMenu();
			return true;
		}
		return true;
	}

	bool handleAddress(ButtonEvent evt, ChordAction chord) {
		uint8_t totalItems = 1 + seenCount;

		if (evt == ButtonEvent::DotPress) {
			if (addrCursor > 0) addrCursor--;
			drawAddress();
			return true;
		}
		if (evt == ButtonEvent::DashPress) {
			if (addrCursor < totalItems - 1) addrCursor++;
			drawAddress();
			return true;
		}
		if (evt == ButtonEvent::SendPress) {
			exitToMenu();
			return true;
		}
		if (evt == ButtonEvent::Chord && chord == ChordAction::Select) {
			if (addrCursor == 0) {
				// Toggle broadcast
				broadcastMode = !broadcastMode;
			} else {
				// Select a target device
				broadcastMode = false;
				targetID = seenDevices[addrCursor - 1];
			}
			drawAddress();
			return true;
		}
		return true;
	}

	bool handleSettings(ButtonEvent evt, ChordAction /*chord*/) {
		if (evt == ButtonEvent::SendPress) {
			exitToMenu();
			return true;
		}
		return true;
	}
}

// ---- Public API ----------------------------------------------------------

void uiSetup() {
	state = UIState::Main;
}

bool uiHandleInput(ButtonEvent evt, ChordAction chord) {
	if (evt == ButtonEvent::None) return false;

	// OpenMenu chord from main screen
	if (state == UIState::Main && evt == ButtonEvent::Chord && chord == ChordAction::OpenMenu) {
		enterMenu();
		return true;
	}

	switch (state) {
		case UIState::Menu:     return handleMenu(evt, chord);
		case UIState::History:  return handleHistory(evt, chord);
		case UIState::Address:  return handleAddress(evt, chord);
		case UIState::Settings: return handleSettings(evt, chord);
		default:                return false;  // Main — not consumed
	}
}

UIState uiGetState() {
	return state;
}

void uiTrackDevice(uint8_t srcID) {
	if (srcID == BROADCAST_ADDR || srcID == 0) return;

	// Update targetID from last sender
	targetID = srcID;

	// Add to seen list if not already present
	for (uint8_t i = 0; i < seenCount; ++i) {
		if (seenDevices[i] == srcID) return;
	}
	if (seenCount < MAX_SEEN_DEVICES) {
		seenDevices[seenCount++] = srcID;
	}
}

bool uiIsBroadcast() {
	return broadcastMode;
}

uint8_t uiGetTargetID() {
	return targetID;
}

void uiAddHistory(const String &msg) {
	if (historyCount < MESSAGE_HISTORY_SIZE) {
		historyBuf[historyCount++] = msg;
	} else {
		for (uint8_t i = 1; i < MESSAGE_HISTORY_SIZE; ++i) {
			historyBuf[i - 1] = historyBuf[i];
		}
		historyBuf[MESSAGE_HISTORY_SIZE - 1] = msg;
	}
}

const String& uiGetHistoryItem(uint8_t idx) {
	return historyBuf[idx];
}

uint8_t uiGetHistoryCount() {
	return historyCount;
}

void uiSetLastRssi(int16_t rssi) {
	lastRssi = rssi;
}
