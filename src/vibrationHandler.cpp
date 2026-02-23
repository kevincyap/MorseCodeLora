#include "vibrationHandler.h"
#include "config.h"

namespace {

enum class VibState : uint8_t {
    Idle,
    Notify,         // initial notification buzz
    NotifyGap,      // pause after notify before morse playback
    SymbolOn,       // dot or dash vibration active
    SymbolGap,      // gap between symbols within a character
    CharGap,        // gap between characters
};

VibState state = VibState::Idle;
unsigned long stateStartMs = 0;
unsigned long stateDurationMs = 0;

String morseBuffer;
uint16_t morsePos = 0;

void motorOn()  { digitalWrite(PIN_VIBRATION, HIGH); }
void motorOff() { digitalWrite(PIN_VIBRATION, LOW);  }

void enterState(VibState newState, unsigned long durationMs) {
    state = newState;
    stateStartMs = millis();
    stateDurationMs = durationMs;
    if (newState == VibState::Notify || newState == VibState::SymbolOn) {
        motorOn();
    } else {
        motorOff();
    }
}

// Advance to the next morse symbol in the buffer.
void advanceMorse() {
    while (morsePos < morseBuffer.length()) {
        char c = morseBuffer[morsePos++];
        if (c == '.') {
            enterState(VibState::SymbolOn, VIB_DOT_MS);
            return;
        } else if (c == '-') {
            enterState(VibState::SymbolOn, VIB_DASH_MS);
            return;
        } else if (c == ' ') {
            enterState(VibState::CharGap, VIB_CHAR_GAP_MS);
            return;
        } else if (c == '/') {
            enterState(VibState::CharGap, VIB_CHAR_GAP_MS * 2);
            return;
        }
        // skip unknown characters
    }
    // End of pattern
    enterState(VibState::Idle, 0);
}

}  // namespace

void vibrationSetup() {
    pinMode(PIN_VIBRATION, OUTPUT);
    motorOff();
}

void vibrationNotify() {
    morseBuffer = "";
    morsePos = 0;
    enterState(VibState::Notify, VIB_NOTIFY_MS);
}

void vibrationPlayMorse(const String &morsePattern) {
    morseBuffer = morsePattern;
    morsePos = 0;
    // Start with notification buzz, then play morse
    enterState(VibState::Notify, VIB_NOTIFY_MS);
}

void vibrationStop() {
    morseBuffer = "";
    morsePos = 0;
    enterState(VibState::Idle, 0);
}

bool vibrationBusy() {
    return state != VibState::Idle;
}

void vibrationUpdate() {
    if (state == VibState::Idle) {
        return;
    }

    if ((millis() - stateStartMs) < stateDurationMs) {
        return;  // still in current state
    }

    switch (state) {
        case VibState::Notify:
            if (morseBuffer.length() > 0) {
                enterState(VibState::NotifyGap, VIB_CHAR_GAP_MS);
            } else {
                enterState(VibState::Idle, 0);
            }
            break;

        case VibState::NotifyGap:
            advanceMorse();
            break;

        case VibState::SymbolOn:
            if (morsePos < morseBuffer.length()) {
                char next = morseBuffer[morsePos];
                if (next == '.' || next == '-') {
                    enterState(VibState::SymbolGap, VIB_SYMBOL_GAP_MS);
                } else {
                    advanceMorse();
                }
            } else {
                enterState(VibState::Idle, 0);
            }
            break;

        case VibState::SymbolGap:
            advanceMorse();
            break;

        case VibState::CharGap:
            advanceMorse();
            break;

        default:
            enterState(VibState::Idle, 0);
            break;
    }
}
