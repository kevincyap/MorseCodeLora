"""
Morse Code Practice Tool — simulates the MorseCodeLora two-button input.

Usage:
  python morse_practice.py           # Write mode (default): compose morse
  python morse_practice.py read      # Read mode: decode displayed morse

Write mode keys:
  y  = dot (.)
  e  = dash (-)
  i  = send / commit message
  o  = backspace (delete last symbol or character)
  space = insert space in message
  Ctrl+C = quit

Read mode:
  Morse code is shown for a random word. Type the decoded letter.
  Press Enter to skip, Ctrl+C to quit.
"""

import sys
import time
import msvcrt
import random
import os

MORSE_TABLE = {
    ".-": "A", "-...": "B", "-.-.": "C", "-..": "D", ".": "E",
    "..-.": "F", "--.": "G", "....": "H", "..": "I", ".---": "J",
    "-.-": "K", ".-..": "L", "--": "M", "-.": "N", "---": "O",
    ".--.": "P", "--.-": "Q", ".-.": "R", "...": "S", "-": "T",
    "..-": "U", "...-": "V", ".--": "W", "-..-": "X", "-.--": "Y",
    "--..": "Z",
    "-----": "0", ".----": "1", "..---": "2", "...--": "3",
    "....-": "4", ".....": "5", "-....": "6", "--...": "7",
    "---..": "8", "----.": "9",
    ".-.-.-": ".", "--..--": ",", "..--..": "?", "-.-.--": "!",
    "-..-.": "/", "-.--.": "(", "-.--.-": ")", ".-...": "&",
    "---...": ":", "-.-.-.": ";", "-...-": "=", ".-.-.": "+",
    "-....-": "-", "..--.-": "_", ".-..-.": '"', "...-..-": "$",
    ".--.-.": "@",
}

CHAR_TO_MORSE = {v: k for k, v in MORSE_TABLE.items()}

CHAR_TIMEOUT = 0.5  # seconds

PRACTICE_WORDS = [
    "HELLO", "WORLD", "SOS", "HELP", "OK", "YES", "NO", "STOP",
    "GO", "WAIT", "MEET", "CAMP", "WATER", "FOOD", "SAFE", "DANGER",
    "NORTH", "SOUTH", "EAST", "WEST", "HOME", "BASE", "SEND", "COPY",
    "OVER", "OUT", "ROGER", "WILCO", "ALPHA", "BRAVO", "DELTA", "ECHO",
    "GOLF", "HOTEL", "LIMA", "OSCAR", "PAPA", "TANGO", "VICTOR", "ZULU",
]

DEFAULT_WORDS_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "practice_words.txt")

MORSE_REF = """  A .-     B -...   C -.-.   D -..    E .      F ..-.
  G --.    H ....   I ..     J .---   K -.-    L .-..
  M --     N -.     O ---    P .--.   Q --.-   R .-.
  S ...    T -      U ..-    V ...-   W .--    X -..-
  Y -.--   Z --..
  0 -----  1 .----  2 ..---  3 ...--  4 ....-  5 .....
  6 -....  7 --...  8 ---..  9 ----."""


def write_mode():
    morse_buf = ""
    decoded = ""
    last_symbol_time = None
    message_log = []

    print("=" * 50)
    print("  Write Mode — compose morse code")
    print("  y = dot  |  e = dash  |  i = send  |  o = bksp  |  space = space")
    print("  Ctrl+C to quit")
    print("=" * 50)
    print(MORSE_REF)
    print()

    def redraw():
        sys.stdout.write(f"\r\033[K  Morse: {morse_buf:<30} Text: {decoded}")
        sys.stdout.flush()

    def commit_char():
        nonlocal morse_buf, decoded
        if morse_buf:
            ch = MORSE_TABLE.get(morse_buf, "?")
            decoded += ch
            morse_buf = ""

    redraw()

    try:
        while True:
            if last_symbol_time and morse_buf:
                if time.time() - last_symbol_time >= CHAR_TIMEOUT:
                    commit_char()
                    last_symbol_time = None
                    redraw()

            if not msvcrt.kbhit():
                time.sleep(0.02)
                continue

            key = msvcrt.getch().decode("utf-8", errors="ignore").lower()

            if key == "y":
                morse_buf += "."
                last_symbol_time = time.time()
                redraw()
            elif key == "e":
                morse_buf += "-"
                last_symbol_time = time.time()
                redraw()
            elif key == "i":
                commit_char()
                if decoded:
                    print(f"\r\033[K  >> Sent: {decoded}")
                    message_log.append(decoded)
                    decoded = ""
                    morse_buf = ""
                    last_symbol_time = None
                else:
                    print(f"\r\033[K  >> (nothing to send)")
                redraw()
            elif key == "o":
                if morse_buf:
                    morse_buf = morse_buf[:-1]
                    last_symbol_time = time.time() if morse_buf else None
                elif decoded:
                    decoded = decoded[:-1]
                redraw()
            elif key == " ":
                commit_char()
                decoded += " "
                last_symbol_time = None
                redraw()

    except KeyboardInterrupt:
        print("\n")
        if message_log:
            print("Session messages:")
            for i, msg in enumerate(message_log, 1):
                print(f"  {i}. {msg}")
        print("Goodbye!")


def load_words(filepath=None):
    path = filepath or DEFAULT_WORDS_FILE
    try:
        with open(path, "r") as f:
            words = [line.strip().upper() for line in f if line.strip()]
        if words:
            return words
    except FileNotFoundError:
        pass
    return PRACTICE_WORDS


def read_mode(words_file=None):
    words = load_words(words_file)
    correct = 0
    total = 0
    skipped = 0

    print("=" * 50)
    print("  Read Mode — decode morse code")
    print("  Type the letter for each morse pattern shown")
    print("  Enter = skip  |  Ctrl+C = quit")
    print("=" * 50)
    print()
    
    try:
        while True:
            word = random.choice(words)
            morse_word = "  ".join(CHAR_TO_MORSE.get(c, "?") for c in word)

            print(f"  Morse:  {morse_word}")
            print(f"  Decode: ", end="", flush=True)

            answer = ""
            for i, ch in enumerate(word):
                ch_morse = CHAR_TO_MORSE.get(ch, "?")
                total += 1

                # Wait for keypress
                while True:
                    if not msvcrt.kbhit():
                        time.sleep(0.02)
                        continue
                    key = msvcrt.getch().decode("utf-8", errors="ignore")
                    if key == "\r" or key == "\n":
                        # Skip rest of word
                        remaining = word[i:]
                        print(f"\033[33m[{remaining}]\033[0m", end="", flush=True)
                        skipped += len(remaining)
                        total += len(remaining) - 1
                        answer += remaining
                        break
                    elif key.upper() == ch:
                        print(f"\033[32m{ch}\033[0m", end="", flush=True)
                        answer += ch
                        correct += 1
                        break
                    else:
                        print(f"\033[31m{key.upper()}\033[0m", end="", flush=True)
                        # Show correct answer
                        print(f"\033[33m({ch})\033[0m", end="", flush=True)
                        answer += ch
                        break

                if len(answer) >= len(word):
                    break

            print()
            pct = (correct / total * 100) if total > 0 else 0
            print(f"  Score: {correct}/{total} ({pct:.0f}%)  Skipped: {skipped}")
            print()

    except KeyboardInterrupt:
        print("\n")
        pct = (correct / total * 100) if total > 0 else 0
        print(f"Final score: {correct}/{total} ({pct:.0f}%)  Skipped: {skipped}")
        print("Goodbye!")


def main():
    mode = sys.argv[1].lower() if len(sys.argv) > 1 else "write"
    words_file = sys.argv[2] if len(sys.argv) > 2 else None

    if mode == "read":
        read_mode(words_file)
    else:
        write_mode()


if __name__ == "__main__":
    main()
