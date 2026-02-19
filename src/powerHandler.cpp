#include "powerHandler.h"
#include "config.h"
#include "displayHandler.h"
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
    VextOFF();

    // Configure button wake pins using GPIO wakeup (EXT1)
    // EXT1 wakes on ANY of the selected pins going LOW
    uint64_t pinMask = (1ULL << PIN_BTN_DOT) |
                       (1ULL << PIN_BTN_DASH) |
                       (1ULL << PIN_BTN_SEND);

    esp_sleep_enable_ext1_wakeup(pinMask, ESP_EXT1_WAKEUP_ANY_LOW);

    esp_deep_sleep_start();
}
