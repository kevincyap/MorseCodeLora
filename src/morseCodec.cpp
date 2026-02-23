#include "morseCodec.h"

namespace {

struct MorseEntry {
    char    character;
    const char *pattern;  // e.g., ".-"
};

// ITU morse code table
const MorseEntry MORSE_TABLE[] = {
    // Letters
    { 'A', ".-" },
    { 'B', "-..." },
    { 'C', "-.-." },
    { 'D', "-.." },
    { 'E', "." },
    { 'F', "..-." },
    { 'G', "--." },
    { 'H', "...." },
    { 'I', ".." },
    { 'J', ".---" },
    { 'K', "-.-" },
    { 'L', ".-.." },
    { 'M', "--" },
    { 'N', "-." },
    { 'O', "---" },
    { 'P', ".--." },
    { 'Q', "--.-" },
    { 'R', ".-." },
    { 'S', "..." },
    { 'T', "-" },
    { 'U', "..-" },
    { 'V', "...-" },
    { 'W', ".--" },
    { 'X', "-..-" },
    { 'Y', "-.--" },
    { 'Z', "--.." },
    // Digits
    { '0', "-----" },
    { '1', ".----" },
    { '2', "..---" },
    { '3', "...--" },
    { '4', "....-" },
    { '5', "....." },
    { '6', "-...." },
    { '7', "--..." },
    { '8', "---.." },
    { '9', "----." },
    // Punctuation
    { '.', ".-.-.-" },
    { ',', "--..--" },
    { '?', "..--.." },
    { '!', "-.-.--" },
    { '/', "-..-." },
    { '(', "-.--." },
    { ')', "-.--.-" },
    { '&', ".-..." },
    { ':', "---..." },
    { ';', "-.-.-." },
    { '=', "-...-" },
    { '+', ".-.-." },
    { '-', "-....-" },
    { '_', "..--.-" },
    { '"', ".-..-." },
    { '$', "...-..-" },
    { '@', ".--.-." },
    { ' ', "/" },       // word separator
};

constexpr size_t MORSE_TABLE_SIZE = sizeof(MORSE_TABLE) / sizeof(MORSE_TABLE[0]);

}  // namespace

char morseToChar(const String &morse) {
    for (size_t i = 0; i < MORSE_TABLE_SIZE; ++i) {
        if (morse == MORSE_TABLE[i].pattern) {
            return MORSE_TABLE[i].character;
        }
    }
    return '\0';
}

String charToMorse(char c) {
    char upper = toupper(c);
    for (size_t i = 0; i < MORSE_TABLE_SIZE; ++i) {
        if (upper == MORSE_TABLE[i].character) {
            return String(MORSE_TABLE[i].pattern);
        }
    }
    return String();
}
