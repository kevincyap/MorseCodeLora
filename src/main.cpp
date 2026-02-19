#include <Wire.h>
#include <Arduino.h>
#include "displayHandler.h"
#include "radioHandler.h"

// Basic OLED + Serial test program for Heltec ESP32 LoRa v4 using RadioLib.

constexpr float kFrequencyMHz = 915.0F;
constexpr bool kReceiveMode = false;  // Set false to transmit demo packets.

double txNumber = 0.0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  displaySetup();
  displayShowMessage("Booting...");

  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  if (!radioInit(kFrequencyMHz)) {
    displayShowMessage("Radio init failed");
    while (true) {
      delay(1000);
    }
  }

  if (kReceiveMode) {
    radioStartReceive();
    displayShowMessage("RX ready");
  }
}

void loop() {
  if (kReceiveMode) {
    String message;
    int16_t rssi = 0;
    if (radioReceiveAvailable(message, rssi, true)) {
      Serial.printf("received \"%s\" with RSSI %d\n", message.c_str(), rssi);
      displayShowMessage(message);
      digitalWrite(LED_BUILTIN, LOW);
    }
    return;
  }

  static unsigned long lastSendMs = 0;
  const unsigned long intervalMs = 1000;

  if (!radioIdle()) {
    return;
  }

  if (millis() - lastSendMs < intervalMs) {
    return;
  }

  digitalWrite(LED_BUILTIN, HIGH);
  txNumber += 1;
  String payload = String(txNumber) + " Hello world nuts";

  TransmitStatus ok = radioTransmit(payload, true);
  Serial.printf("sending packet \"%s\" %s\n", payload.c_str(), ok == TransmitStatus::Ok ? "ok" : "failed");
  displayShowMessage(ok == TransmitStatus::Ok ? "OK: " + payload : "Fail: " + payload);

  digitalWrite(LED_BUILTIN, LOW);
  lastSendMs = millis();
}