#include "inputHandler.h"

namespace {

struct ButtonState {
    uint8_t pin;
    uint8_t mask;
    bool    pressed;
    bool    lastRaw;
    unsigned long lastChangeMs;
};

ButtonState buttons[] = {
    { PIN_BTN_DOT,  BTN_MASK_DOT,  false, false, 0 },
    { PIN_BTN_DASH, BTN_MASK_DASH, false, false, 0 },
    { PIN_BTN_SEND, BTN_MASK_SEND, false, false, 0 },
};
constexpr uint8_t NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

// Chord detection state
uint8_t     activeMask      = 0;       // bitmask of currently-held buttons
unsigned long firstPressMs  = 0;       // timestamp of first button in potential chord
bool        chordEmitted    = false;   // true once a chord fires for this press group
ChordAction lastChord       = ChordAction::None;

// Debounce a single button. Returns true on a fresh press edge.
bool debounce(ButtonState &btn) {
    bool raw = (digitalRead(btn.pin) == LOW);  // active LOW
    if (raw != btn.lastRaw) {
        btn.lastChangeMs = millis();
        btn.lastRaw = raw;
    }
    if ((millis() - btn.lastChangeMs) >= DEBOUNCE_MS) {
        bool wasPressed = btn.pressed;
        btn.pressed = btn.lastRaw;
        if (btn.pressed && !wasPressed) {
            return true;  // fresh press
        }
    }
    return false;
}

// Look up chord in mapping table (wider masks checked first by table order).
ChordAction lookupChord(uint8_t mask) {
    for (uint8_t i = 0; i < CHORD_MAP_SIZE; ++i) {
        if (CHORD_MAP[i].buttonMask == mask) {
            return CHORD_MAP[i].action;
        }
    }
    return ChordAction::None;
}

}  // namespace

void inputSetup() {
    for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
        pinMode(buttons[i].pin, INPUT_PULLUP);
    }
}

ButtonEvent inputUpdate() {
    // Update debounce for all buttons, track new presses
    bool anyNewPress = false;
    for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
        if (debounce(buttons[i])) {
            anyNewPress = true;
        }
    }

    // Build current held mask
    uint8_t heldMask = 0;
    for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
        if (buttons[i].pressed) {
            heldMask |= buttons[i].mask;
        }
    }

    // If a new button was just pressed, start/extend the chord window
    if (anyNewPress) {
        if (activeMask == 0) {
            firstPressMs = millis();
            chordEmitted = false;
        }
        activeMask = heldMask;
    }

    // While within chord window, wait for more buttons
    if (activeMask != 0 && !chordEmitted) {
        if ((millis() - firstPressMs) < CHORD_WINDOW_MS) {
            activeMask = heldMask;  // accumulate
            return ButtonEvent::None;  // still waiting
        }

        // Chord window expired — evaluate
        ChordAction action = lookupChord(activeMask);
        if (action != ChordAction::None) {
            chordEmitted = true;
            lastChord = action;
            return ButtonEvent::Chord;
        }

        // Not a chord — emit the single-button press
        chordEmitted = true;  // prevent re-evaluation
        if (activeMask & BTN_MASK_DOT)  return ButtonEvent::DotPress;
        if (activeMask & BTN_MASK_DASH) return ButtonEvent::DashPress;
        if (activeMask & BTN_MASK_SEND) return ButtonEvent::SendPress;
    }

    // Reset when all buttons released
    if (heldMask == 0 && activeMask != 0) {
        activeMask = 0;
        chordEmitted = false;
    }

    return ButtonEvent::None;
}

ChordAction inputLastChord() {
    return lastChord;
}
