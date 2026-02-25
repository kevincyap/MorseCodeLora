#pragma once

// =============================================================================
// Chord Mappings — Edit this file to remap button chords
// =============================================================================

#include <Arduino.h>
#include "config.h"

enum class ChordAction : uint8_t {
    None,
    OpenMenu,            // enter the page menu
    Backspace,           // delete last morse symbol or character (main screen)
    Select,              // select/confirm in menu pages
    ForceSleep,          // immediately enter deep sleep
};

struct ChordMapping {
    uint8_t     buttonMask;  // OR of BTN_MASK_* constants
    ChordAction action;
};

// Default chord table — order doesn't matter; first match wins.
// Wider chords (more buttons) are checked first by the input handler.
constexpr ChordMapping CHORD_MAP[] = {
    { BTN_MASK_DOT | BTN_MASK_DASH | BTN_MASK_SEND, ChordAction::ForceSleep  },
    { BTN_MASK_DOT | BTN_MASK_SEND,                  ChordAction::OpenMenu    },
    { BTN_MASK_DOT | BTN_MASK_DASH,                  ChordAction::Select      },
    // Dash+Send is currently unassigned.
};

constexpr uint8_t CHORD_MAP_SIZE = sizeof(CHORD_MAP) / sizeof(CHORD_MAP[0]);
