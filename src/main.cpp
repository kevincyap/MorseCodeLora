#include <Wire.h>
#include <Arduino.h>
#include "config.h"
#include "displayHandler.h"
#include "radioHandler.h"
#include "inputHandler.h"
#include "vibrationHandler.h"
#include "morseCodec.h"
#include "packetProtocol.h"
#include "deviceIdentity.h"
#include "chordMappings.h"
#include "powerHandler.h"

// ---- State ------------------------------------------------------------------
static uint8_t localID = 0;
static uint8_t seqNum  = 0;

// Compose state
static String morseBuffer;      // current character's dot/dash sequence
static String decodedText;      // fully decoded characters so far
static String fullMorseString;  // all morse for the message (space-separated chars)
static unsigned long lastSymbolMs = 0;
static bool composing = false;

// Addressing
static bool broadcastMode = true;
static uint8_t targetID = BROADCAST_ADDR;

// Message history
static String messageHistory[MESSAGE_HISTORY_SIZE];
static uint8_t historyCount = 0;
static uint8_t historyViewIdx = 0;
static bool viewingHistory = false;

// ---- Helpers ----------------------------------------------------------------

static void addToHistory(const String &msg) {
    if (historyCount < MESSAGE_HISTORY_SIZE) {
        messageHistory[historyCount++] = msg;
    } else {
        for (uint8_t i = 1; i < MESSAGE_HISTORY_SIZE; ++i) {
            messageHistory[i - 1] = messageHistory[i];
        }
        messageHistory[MESSAGE_HISTORY_SIZE - 1] = msg;
    }
}

static void commitCurrentChar() {
    if (morseBuffer.length() == 0) return;
    char c = morseToChar(morseBuffer);
    if (c != '\0') {
        decodedText += c;
        if (fullMorseString.length() > 0) fullMorseString += ' ';
        fullMorseString += morseBuffer;
    }
    morseBuffer = "";
}

static void prepareSerialMessage(const String &text) {
    decodedText = text;
    decodedText.trim();

    morseBuffer = "";
    fullMorseString = "";

    for (size_t i = 0; i < decodedText.length(); ++i) {
        String m = charToMorse(decodedText[i]);
        if (m.length() > 0) {
            if (fullMorseString.length() > 0) fullMorseString += ' ';
            fullMorseString += m;
        }
    }
}

static void sendMessage() {
    commitCurrentChar();
    if (decodedText.length() == 0) return;

    Packet pkt;
    pkt.srcID  = localID;
    pkt.dstID  = broadcastMode ? BROADCAST_ADDR : targetID;
    pkt.seqNum = seqNum++;
    pkt.flags  = broadcastMode ? PACKET_FLAG_BROADCAST : PACKET_FLAG_ACK_REQ;
    pkt.text   = decodedText;

    displaySending("Sending...");
    TransmitStatus status = radioTransmitPacket(pkt, !broadcastMode, localID);

    switch (status) {
        case TransmitStatus::Ok:
            displaySending("Sent");
            break;
        case TransmitStatus::NoAck:
            displaySending("No ACK");
            break;
        case TransmitStatus::Failed:
            displaySending("Failed");
            break;
        case TransmitStatus::AlreadyTransmitting:
            displaySending("Already Transmitting");
            break;
        case TransmitStatus::NoPacket:
            displaySending("No Packet");
            break;
        case TransmitStatus::TransmitFailed:
            displaySending("Transmit Failed");
            break;
        default:
            displaySending("Error" + String(static_cast<uint8_t>(status)));
            break;
    }

    addToHistory("TX: " + decodedText);
    delay(800);

    // Reset compose state
    morseBuffer = "";
    decodedText = "";
    fullMorseString = "";
    composing = false;
    displayIdle("Ready", "Sent: " + pkt.text);
}

static void handleBackspace() {
    if (morseBuffer.length() > 0) {
        morseBuffer.remove(morseBuffer.length() - 1);
    } else if (decodedText.length() > 0) {
        decodedText.remove(decodedText.length() - 1);
        // Also trim fullMorseString (remove last char's morse)
        int lastSpace = fullMorseString.lastIndexOf(' ');
        fullMorseString = (lastSpace >= 0) ? fullMorseString.substring(0, lastSpace) : "";
    }
    displayComposing(fullMorseString + (morseBuffer.length() > 0 ? " " + morseBuffer : ""), decodedText);
}

static void handleScrollHistory() {
    if (historyCount == 0) {
        displayIdle("No messages", "");
        return;
    }
    if (!viewingHistory) {
        viewingHistory = true;
        historyViewIdx = historyCount - 1;
    } else {
        historyViewIdx = (historyViewIdx > 0) ? historyViewIdx - 1 : historyCount - 1;
    }
    displayIdle("History " + String(historyViewIdx + 1) + "/" + String(historyCount),
                messageHistory[historyViewIdx] + (viewingHistory ? " <" : ""));
}

// ---- Setup & Loop -----------------------------------------------------------

void setup() {
    Serial.begin(115200);

    displaySetup();
    displayShowMessage("Booting...");

    inputSetup();
    vibrationSetup();
    powerSetup();

    localID = deviceGetID();
    Serial.printf("Device ID: 0x%02X  Name: %s\n", localID, deviceGetName().c_str());
    Serial.println("Serial commands: name:<text>  id:<hex>  send:<text>");
    Serial.println("Tip: any plain text line (without ':') is sent as a message.");

    if (!radioInit()) {
        displayShowMessage("Radio FAIL");
        while (true) delay(1000);
    }

    WakeReason wake = powerWakeReason();
    if (wake == WakeReason::Radio) {
        // Woke from radio RX — check for buffered packet immediately
        Serial.println("Woke from radio RX");
        displayIdle("RX Wake  ID:" + String(localID, HEX), "");
    } else if (wake == WakeReason::Button) {
        Serial.println("Woke from button press");
        displayIdle("Ready  ID:" + String(localID, HEX), "");
    } else {
        displayIdle("Ready  ID:" + String(localID, HEX), "");
    }
}

void loop() {
    ButtonEvent evt = inputUpdate();
    vibrationUpdate();

    // ---- Handle button/chord events -----------------------------------------
    if (evt != ButtonEvent::None) {
        // Exit history view on any press EXCEPT scroll chord
        if (!(evt == ButtonEvent::Chord && inputLastChord() == ChordAction::ScrollHistory)) {
            viewingHistory = false;
        }
    }

    switch (evt) {
        case ButtonEvent::DotPress:
            composing = true;
            morseBuffer += '.';
            lastSymbolMs = millis();
            displayComposing(fullMorseString + (morseBuffer.length() > 0 ? " " + morseBuffer : ""), decodedText);
            break;

        case ButtonEvent::DashPress:
            composing = true;
            morseBuffer += '-';
            lastSymbolMs = millis();
            displayComposing(fullMorseString + (morseBuffer.length() > 0 ? " " + morseBuffer : ""), decodedText);
            break;

        case ButtonEvent::SendPress:
            sendMessage();
            break;

        case ButtonEvent::Chord: {
            ChordAction action = inputLastChord();
            switch (action) {
                case ChordAction::Backspace:
                    handleBackspace();
                    break;
                case ChordAction::ScrollHistory:
                    handleScrollHistory();
                    break;
                case ChordAction::ToggleAddressing:
                    broadcastMode = !broadcastMode;
                    displayIdle(broadcastMode ? "Broadcast ON" : "Addr: " + String(targetID, HEX), "");
                    break;
                case ChordAction::ForceSleep:
                    powerDeepSleep();
                    break;
                default:
                    break;
            }
            break;
        }

        default:
            break;
    }

    // ---- Auto-decode character after timeout --------------------------------
    if (composing && morseBuffer.length() > 0 && (millis() - lastSymbolMs) >= CHAR_TIMEOUT_MS) {
        commitCurrentChar();
        displayComposing(fullMorseString + (morseBuffer.length() > 0 ? " " + morseBuffer : ""), decodedText);
    }

    // ---- Check for incoming packets -----------------------------------------
    Packet rxPkt;
    int16_t rssi = 0;
    if (radioReceivePacket(rxPkt, rssi, localID)) {
        if (!rxPkt.isAck()) {
            // Send ACK if requested
            if (rxPkt.isAckRequest()) {
                Packet ack = packetMakeAck(rxPkt, localID);
                radioTransmitPacket(ack);
            }

            // Track last sender as default target for addressed mode
            if (rxPkt.srcID != BROADCAST_ADDR) {
                targetID = rxPkt.srcID;
            }

            // Display and vibrate
            displayReceiving(rxPkt.text, rssi);
            addToHistory("RX: " + rxPkt.text);

            // Regenerate morse from ASCII for vibration replay
            String morsePattern;
            for (size_t i = 0; i < rxPkt.text.length(); ++i) {
                String m = charToMorse(rxPkt.text[i]);
                if (m.length() > 0) {
                    if (morsePattern.length() > 0) morsePattern += ' ';
                    morsePattern += m;
                }
            }

            if (morsePattern.length() > 0) {
                vibrationPlayMorse(morsePattern);
            } else {
                vibrationNotify();
            }

            Serial.printf("RX [%02X->%02X]: \"%s\" RSSI=%d\n", rxPkt.srcID, rxPkt.dstID, rxPkt.text.c_str(), rssi);
        }
    }

    // ---- Power management ---------------------------------------------------
    bool activity = (evt != ButtonEvent::None) || rxPkt.text.length() > 0;
    powerUpdate(activity || composing);
    if (deviceGetID() == 0x42)
    {
        prepareSerialMessage(String(millis() % 10));
        sendMessage();
        delay(5000);
    }
    // ---- Serial commands (name:<text>, id:<hex>, send:<text>) ----------------
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.length() == 0) {
            // Ignore empty lines.
        } else if (cmd.startsWith("name:")) {
            String newName = cmd.substring(5);
            newName.trim();
            deviceSetName(newName);
            Serial.printf("Name set to: %s\n", deviceGetName().c_str());
            displayIdle(deviceGetName(), "");
        } else if (cmd.startsWith("id:")) {
            uint8_t newID = (uint8_t)strtol(cmd.substring(3).c_str(), nullptr, 16);
            if (newID > 0 && newID < 0xFF) {
                deviceSetID(newID);
                localID = newID;
                Serial.printf("ID set to: 0x%02X\n", localID);
                displayIdle("ID:" + String(localID, HEX) + " " + deviceGetName(), "");
            } else {
                Serial.println("Invalid ID (must be 01-FE)");
            }
        } else if (cmd.startsWith("send:")) {
            String serialText = cmd.substring(5);
            serialText.trim();
            prepareSerialMessage(serialText);
            sendMessage();
        } else if (cmd.indexOf(':') < 0) {
            // Plain text fallback for quick serial monitor use.
            prepareSerialMessage(cmd);
            sendMessage();
        } else {
            Serial.println("Unknown command. Use name:<text>, id:<hex>, or send:<text>");
        }
    }
}