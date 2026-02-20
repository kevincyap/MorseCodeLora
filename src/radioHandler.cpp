#include "radioHandler.h"
#include <RadioLib.h>
#include <Arduino.h>

namespace {
	volatile bool receivedFlag = false;
	bool transmitting = false;
	volatile bool enableInterrupt = true;

	int16_t lastRssi = 0;

	SX1262 radio = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);

	portMUX_TYPE radioMux = portMUX_INITIALIZER_UNLOCKED;

	void setFlag() {
		if (!enableInterrupt) {
			return;
		}
		portENTER_CRITICAL_ISR(&radioMux);
		receivedFlag = true;
		portEXIT_CRITICAL_ISR(&radioMux);
	}

	// Transmit raw bytes (internal helper).
	bool rawTransmit(const uint8_t *data, uint8_t len) {
		enableInterrupt = false;
		transmitting = true;
		int state = radio.transmit(data, len);
		transmitting = false;
		enableInterrupt = true;
		radio.startReceive();
		return state == RADIOLIB_ERR_NONE;
	}
}

bool radioInit() {
	int state = radio.begin(
		LORA_FREQUENCY_MHZ,
		LORA_BANDWIDTH_KHZ,
		LORA_SPREADING_FACTOR,
		LORA_CODING_RATE,
		LORA_SYNC_WORD,
		LORA_TX_POWER_DBM,
		LORA_PREAMBLE_LENGTH
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

bool radioReceivePacket(Packet &outPkt, int16_t &outRssi, uint8_t localID) {
	portENTER_CRITICAL(&radioMux);
	bool hadFlag = receivedFlag;
	receivedFlag = false;
	portEXIT_CRITICAL(&radioMux);

	if (!hadFlag) {
		return false;
	}

	enableInterrupt = false;

	uint8_t buf[256];
	size_t len = radio.getPacketLength();
	if (len == 0 || len > sizeof(buf)) {
		enableInterrupt = true;
		radio.startReceive();
		return false;
	}

	int state = radio.readData(buf, len);
	if (state != RADIOLIB_ERR_NONE) {
		Serial.printf("Radio readData failed, code %d\n", state);
		enableInterrupt = true;
		radio.startReceive();
		return false;
	}

	lastRssi = radio.getRSSI();
	enableInterrupt = true;
	radio.startReceive();

	Packet pkt;
	if (!packetDeserialize(buf, len, pkt)) {
		return false;
	}

	// Address filter: accept broadcast or packets addressed to us
	if (pkt.dstID != BROADCAST_ADDR && pkt.dstID != localID) {
		return false;
	}

	outPkt = pkt;
	outRssi = lastRssi;
	return true;
}

TransmitStatus radioTransmitPacket(const Packet &pkt, bool waitAck, uint8_t localID) {
	if (transmitting) {
		return TransmitStatus::AlreadyTransmitting;
	}

	uint8_t buf[256];
	uint8_t len = packetSerialize(pkt, buf, static_cast<uint16_t>(sizeof(buf)));
	if (len == 0) {
		return TransmitStatus::NoPacket;
	}

	if (!rawTransmit(buf, len)) {
		return TransmitStatus::TransmitFailed;
	}

	if (waitAck && !pkt.isBroadcast()) {
		unsigned long startMs = millis();
		while (millis() - startMs < ACK_TIMEOUT_MS) {
			Packet ackPkt;
			int16_t rssi;
			if (radioReceivePacket(ackPkt, rssi, localID)) {
				if (ackPkt.isAck() && ackPkt.seqNum == pkt.seqNum && ackPkt.srcID == pkt.dstID) {
					return TransmitStatus::Ok;
				}
			}
			delay(10);
		}
		return TransmitStatus::NoAck;
	}

	return TransmitStatus::Ok;
}

bool radioIdle() {
	return !transmitting;
}

int16_t radioLastRssi() {
	return lastRssi;
}

bool radioStartDutyCycle() {
	enableInterrupt = true;

#if DUTY_CYCLE_RX_MS > 0 && DUTY_CYCLE_SLEEP_MS > 0
	// Manual tuning: use config.h values (microseconds to RadioLib)
	int state = radio.startReceiveDutyCycle(
		DUTY_CYCLE_RX_MS * 1000UL,
		DUTY_CYCLE_SLEEP_MS * 1000UL
	);
#else
	// Auto mode: RadioLib calculates optimal windows from preamble config
	int state = radio.startReceiveDutyCycleAuto(LORA_PREAMBLE_LENGTH, 8);
#endif

	if (state != RADIOLIB_ERR_NONE) {
		Serial.printf("Radio duty cycle failed, code %d\n", state);
		return false;
	}
	return true;
}