#include "powerHandler.h"
#include "config.h"
#include "displayHandler.h"
#include "radioHandler.h"
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_sleep.h>

namespace {
    unsigned long lastActivityMs = 0;
    bool displayDimmed = false;
}

void powerSetup() {
    // Disable WiFi & BT to save power
    esp_wifi_stop();
    esp_bt_controller_disable();

    lastActivityMs = millis();
    displayDimmed = false;
}

WakeReason powerWakeReason() {
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    switch (cause) {
        case ESP_SLEEP_WAKEUP_EXT0:
            return WakeReason::Radio;
        case ESP_SLEEP_WAKEUP_EXT1:
            return WakeReason::Button;
        default:
            return WakeReason::PowerOn;
    }
}

void powerUpdate(bool activity) {
    if (activity) {
        lastActivityMs = millis();
        if (displayDimmed) {
            displayDim(false);
            displayDimmed = false;
        }
        return;
    }

    unsigned long idleMs = millis() - lastActivityMs;

    // Stage 1: dim display
    if (!displayDimmed && idleMs >= DISPLAY_DIM_MS) {
        displayDim(true);
        displayDimmed = true;
    }

    // Stage 2: deep sleep
    if (idleMs >= DEEP_SLEEP_MS) {
        powerDeepSleep();
    }
}

void powerDeepSleep() {
    displayShowMessage("Sleeping...");
    delay(300);

    // Start radio duty cycle before sleeping — SX1262 will autonomously
    // wake/listen/sleep and assert DIO1 on packet reception.
    radioStartDutyCycle();

    // Disable external PA for deep sleep
    radioPaSleep();

    VextOFF();

    // Configure wake pins: buttons + DIO1 (radio RX interrupt)
    // DIO0 in our pin map is actually DIO1 on the SX1262 (GPIO 14, RTC-capable)
    uint64_t pinMask = (1ULL << PIN_BTN_DOT) |
                       (1ULL << PIN_BTN_DASH) |
                       (1ULL << PIN_BTN_SEND);

    // Buttons wake on LOW (active LOW with pull-up)
    esp_err_t err = esp_sleep_enable_ext1_wakeup(pinMask, ESP_EXT1_WAKEUP_ANY_LOW);
    if (err != ESP_OK) {
        Serial.printf("EXT1 wake config failed: %d\n", err);
        VextON();
        displayShowMessage("Wake cfg fail!");
        return;
    }

    // DIO1 (radio) wakes on HIGH (asserted when packet received)
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(DIO0), 1);

    // Wait for all buttons to be released so held keys don't
    // immediately trigger the EXT1 ANY_LOW wake source.
    while (digitalRead(PIN_BTN_DOT) == LOW ||
           digitalRead(PIN_BTN_DASH) == LOW ||
           digitalRead(PIN_BTN_SEND) == LOW) {
        delay(10);
    }
    delay(50);  // debounce settling

    esp_deep_sleep_start();
}
