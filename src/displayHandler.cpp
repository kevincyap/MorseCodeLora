#include "displayHandler.h"
#include "HT_SSD1306Wire.h"
#include <Arduino.h>

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

void displaySetup() {
	VextON();
	delay(100);
	display.init();
	display.setLogBuffer(5, 30);
	display.setContrast(255);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.clear();
	display.display();
	display.screenRotate(ANGLE_0_DEGREE);
	display.setFont(ArialMT_Plain_10);
}

void displayShowMessage(const String &text) {
	display.clear();
	display.println(text);
	display.drawLogBuffer(0, 0);
	display.display();
}
