#pragma once

#include <Arduino.h>

// Non-blocking vibration motor driver.
// Call vibrationSetup() once, then vibrationUpdate() every loop iteration.

void vibrationSetup();

// Trigger a short notification buzz.
void vibrationNotify();

// Queue a morse pattern for playback (e.g., ".- -... -.-.").
// Characters separated by space, words by '/'.
void vibrationPlayMorse(const String &morsePattern);

// Cancel any in-progress vibration.
void vibrationStop();

// Returns true while a vibration sequence is playing.
bool vibrationBusy();

// Must be called every loop iteration to drive the state machine.
void vibrationUpdate();
