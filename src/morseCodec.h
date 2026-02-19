#pragma once

#include <Arduino.h>

// Morse codec: bidirectional conversion between characters and dot/dash strings.
// Dot = '.', Dash = '-', space between characters is implicit.

// Decode a dot/dash sequence (e.g., ".-") to its character, or '\0' if unknown.
char morseToChar(const String &morse);

// Encode a character to its dot/dash sequence, or "" if not encodable.
String charToMorse(char c);
