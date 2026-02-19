#include "deviceIdentity.h"
#include <Preferences.h>

namespace {
    Preferences prefs;
    const char *NVS_NAMESPACE = "mcl";
    const char *NVS_KEY_ID    = "devid";
    uint8_t cachedID = 0;
    bool loaded = false;
}

uint8_t deviceGetID() {
    if (!loaded) {
        prefs.begin(NVS_NAMESPACE, true);  // read-only
        if (prefs.isKey(NVS_KEY_ID)) {
            cachedID = prefs.getUChar(NVS_KEY_ID, 0);
        } else {
            // First boot: assign random ID (1-254, avoid 0 and 0xFF broadcast)
            prefs.end();
            randomSeed(esp_random());
            cachedID = random(1, 255);
            prefs.begin(NVS_NAMESPACE, false);
            prefs.putUChar(NVS_KEY_ID, cachedID);
        }
        prefs.end();
        loaded = true;
    }
    return cachedID;
}

void deviceSetID(uint8_t id) {
    cachedID = id;
    loaded = true;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putUChar(NVS_KEY_ID, id);
    prefs.end();
}
