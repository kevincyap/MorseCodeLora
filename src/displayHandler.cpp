#include "displayHandler.h"
#include "HT_SSD1306Wire.h"
#include <Arduino.h>

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

void displaySetup() {
	VextON();
	delay(100);
	display.init();
	display.setContrast(255);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.clear();
	display.display();
	display.screenRotate(ANGLE_0_DEGREE);
	display.setFont(ArialMT_Plain_10);
}

void displaySetScreen(DisplayScreen /*screen*/) {
	// Reserved for future state tracking
}

void displayIdle(const String &status, const String &lastMsg) {
	display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.drawString(64, 0, status);
	if (lastMsg.length() > 0) {
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawString(0, 20, lastMsg);
	}
	display.display();
}

void displayComposing(const String &morseInput, const String &decodedPreview, const String &rxNotify) {
	display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);

	// Top: label + raw morse symbols
	display.drawString(0, 0, "Morse:");
	display.drawString(0, 12, morseInput);

	// Middle: decoded text preview
	display.drawHorizontalLine(0, 30, 128);
	display.drawString(0, 34, decodedPreview);

	// Bottom: RX notification banner (if any)
	if (rxNotify.length() > 0) {
		display.drawHorizontalLine(0, 50, 128);
		display.drawString(0, 52, "RX:" + rxNotify);
	}

	display.display();
}

void displaySending(const String &status) {
	display.clear();
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.drawString(64, 20, status);
	display.display();
}

void displayReceiving(const String &text, int16_t rssi) {
	display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 0, "RX:");
	display.drawString(0, 14, text);
	if (rssi != 0) {
		display.setTextAlignment(TEXT_ALIGN_RIGHT);
		display.drawString(128, 54, String(rssi) + "dBm");
	}
	display.display();
}

void displayDim(bool dim) {
	display.setContrast(dim ? 0 : 255);
}

void displayShowMessage(const String &text) {
	display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.drawString(64, 26, text);
	display.display();
}

void displayMenu(const char* const items[], uint8_t count, uint8_t cursor) {
	display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);

	// Title bar
	display.drawString(0, 0, "MENU");
	display.drawHorizontalLine(0, 12, 128);

	// List items with cursor
	for (uint8_t i = 0; i < count && i < 4; ++i) {
		int16_t y = 15 + i * 13;
		String prefix = (i == cursor) ? "> " : "  ";
		display.drawString(0, y, prefix + String(items[i]));
	}
	display.display();
}

void displayPage(const String &title, const String &content) {
	display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);

	// Title bar
	display.drawString(0, 0, title);
	display.drawHorizontalLine(0, 12, 128);

	// Content body — split by newlines, draw up to 4 lines
	int16_t y = 15;
	int start = 0;
	for (int i = 0; i <= (int)content.length() && y < 64; ++i) {
		if (i == (int)content.length() || content[i] == '\n') {
			display.drawString(0, y, content.substring(start, i));
			y += 13;
			start = i + 1;
		}
	}
	display.display();
}
