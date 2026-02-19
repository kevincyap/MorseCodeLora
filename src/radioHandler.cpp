#include "radioHandler.h"
#include "displayHandler.h"
#include <RadioLib.h>
#include <Arduino.h>

namespace {
	constexpr float kBandwidthKHz = 125.0F;
	constexpr uint8_t kSpreadingFactor = 7;   // SF7
	constexpr uint8_t kCodingRate = 5;        // 4/5
	constexpr int8_t kOutputPowerDbm = 5;     // matches prior config
	constexpr uint16_t kPreambleLength = 8;

	volatile bool receivedFlag = false;
	volatile bool transmitting = false;
	volatile bool enableInterrupt = true;

	String lastMessage;
	int16_t lastRssi = 0;

	SX1262 radio = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);

	void setFlag() {
		if (!enableInterrupt) {
			return;
		}
		receivedFlag = true;
	}
}

bool radioInit(float frequencyMHz) {
	int state = radio.begin(
		frequencyMHz,
		kBandwidthKHz,
		kSpreadingFactor,
		kCodingRate,
		RADIOLIB_SX126X_SYNC_WORD_PRIVATE,
		kOutputPowerDbm
	);

	if (state != RADIOLIB_ERR_NONE) {
		Serial.printf("Radio init failed, code %d\n", state);
		return false;
	}

	radio.setDio1Action(setFlag);

	state = radio.startReceive();
	if (state != RADIOLIB_ERR_NONE) {
		Serial.printf("Radio startReceive failed, code %d\n", state);
		return false;
	}

	return true;
}

bool radioStartReceive() {
	enableInterrupt = true;
	int state = radio.startReceive();
	if (state != RADIOLIB_ERR_NONE) {
		Serial.printf("Radio startReceive failed, code %d\n", state);
		return false;
	}
	return true;
}

bool radioReceiveAvailable(String &outMessage, int16_t &outRssi, bool sendAck) {
	if (!receivedFlag) {
		return false;
	}

	enableInterrupt = false;
	receivedFlag = false;

	String incoming;
	int state = radio.readData(incoming);
	if (state == RADIOLIB_ERR_NONE) {
		lastMessage = incoming;
		lastRssi = radio.getRSSI();
		outMessage = lastMessage;
		outRssi = lastRssi;
		enableInterrupt = true;
        if (sendAck) {
            String ack = "ACK";
            radioTransmit(ack, false);
        }
		radio.startReceive();
		return true;
	}

	Serial.printf("Radio readData failed, code %d\n", state);
	enableInterrupt = true;
	radio.startReceive();
	return false;
}

 TransmitStatus radioTransmit(String &payload, bool shouldWaitForAck) {

    if (transmitting) {
        return TransmitStatus::Failed;
    }
	enableInterrupt = false;
    transmitting = true;

	int state = radio.transmit(payload);

	enableInterrupt = true;
    transmitting = false;
	radio.startReceive();

	if (state != RADIOLIB_ERR_NONE) {
		Serial.printf("Fail %d: %s\n", state, payload.c_str());
		return TransmitStatus::Failed;
	}

	if (shouldWaitForAck) {
		if (!waitForAck(2000)) {
            displayShowMessage("NoAck: " + payload);
			Serial.println("ACK not received");
			return TransmitStatus::NoAck;
		}
	}

	return TransmitStatus::Ok;
}

bool waitForAck(unsigned long timeoutMs) {
    unsigned long startMs = millis();
    while (millis() - startMs < timeoutMs) {
        String message;
        int16_t rssi = 0;
        if (radioReceiveAvailable(message, rssi)) {
            if (message == "ACK") {
                return true;
            }
        }
        delay(10);
    }
    return false;
}

bool radioIdle() {
	return !transmitting;
}