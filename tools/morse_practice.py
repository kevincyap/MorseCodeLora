"""
Morse Code Practice Tool — simulates the MorseCodeLora two-button input.

Keys:
  y  = dot (.)
  e  = dash (-)
  i  = send / commit message
  o  = backspace (delete last symbol or character)
  space = insert space in message
  Ctrl+C = quit

Characters auto-decode after 0.5s of inactivity.
"""

import sys
import time
import msvcrt

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

CHAR_TIMEOUT = 0.5  # seconds

def main():
    morse_buf = ""
    decoded = ""
    last_symbol_time = None
    message_log = []

    print("=" * 50)
    print("  Morse Code Practice Tool")
    print("  y = dot (.)  |  e = dash (-)  |  i = send  |  o = bksp  |  space = space")
    print("  Ctrl+C to quit")
    print("=" * 50)
    print()
    print("  A .-     B -...   C -.-.   D -..    E .      F ..-.")
    print("  G --.    H ....   I ..     J .---   K -.-    L .-..")
    print("  M --     N -.     O ---    P .--.   Q --.-   R .-.")
    print("  S ...    T -      U ..-    V ...-   W .--    X -..-")
    print("  Y -.--   Z --..")
    print("  0 -----  1 .----  2 ..---  3 ...--  4 ....-  5 .....")
    print("  6 -....  7 --...  8 ---..  9 ----.")
    print()

    def redraw():
        current_morse = morse_buf
        sys.stdout.write(f"\r\033[K  Morse: {current_morse:<30} Text: {decoded}")
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
            # Check for timeout
            if last_symbol_time and morse_buf:
                if time.time() - last_symbol_time >= CHAR_TIMEOUT:
                    commit_char()
                    last_symbol_time = None
                    redraw()

            if not msvcrt.kbhit():
                time.sleep(0.02)
                continue

            key = msvcrt.getch().decode("utf-8", errors="ignore").lower()

            if key == "y":  # dot
                morse_buf += "."
                last_symbol_time = time.time()
                redraw()
            elif key == "e":  # dash
                morse_buf += "-"
                last_symbol_time = time.time()
                redraw()
            elif key == "i":  # send
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
            elif key == "o":  # backspace
                if morse_buf:
                    morse_buf = morse_buf[:-1]
                    last_symbol_time = time.time() if morse_buf else None
                elif decoded:
                    decoded = decoded[:-1]
                redraw()
            elif key == " ":  # space
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


if __name__ == "__main__":
    main()
