#include "deviceIdentity.h"
#include <Preferences.h>

namespace {
    Preferences prefs;
    const char *NVS_NAMESPACE = "mcl";
    const char *NVS_KEY_ID    = "devid";
    const char *NVS_KEY_NAME  = "devname";
    constexpr uint8_t MAX_NAME_LEN = 16;
    uint8_t cachedID = 0;
    String  cachedName;
    bool loaded = false;
    bool nameLoaded = false;
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

String deviceGetName() {
    if (!nameLoaded) {
        prefs.begin(NVS_NAMESPACE, true);
        if (prefs.isKey(NVS_KEY_NAME)) {
            cachedName = prefs.getString(NVS_KEY_NAME, "");
        }
        prefs.end();
        if (cachedName.length() == 0) {
            cachedName = "Device " + String(deviceGetID(), HEX);
        }
        nameLoaded = true;
    }
    return cachedName;
}

void deviceSetName(const String &name) {
    cachedName = name.substring(0, MAX_NAME_LEN);
    nameLoaded = true;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putString(NVS_KEY_NAME, cachedName);
    prefs.end();
}
