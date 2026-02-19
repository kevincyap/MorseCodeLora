#pragma once

#include <Arduino.h>
#include "config.h"
#include "chordMappings.h"

// Button event types returned by inputUpdate().
enum class ButtonEvent : uint8_t {
    None,
    DotPress,
    DashPress,
    SendPress,
    Chord,       // check inputLastChord() for the action
};

// Call once in setup().
void inputSetup();

// Call every loop iteration. Returns the highest-priority event this tick.
ButtonEvent inputUpdate();

// After inputUpdate() returns Chord, call this to get the chord action.
ChordAction inputLastChord();
