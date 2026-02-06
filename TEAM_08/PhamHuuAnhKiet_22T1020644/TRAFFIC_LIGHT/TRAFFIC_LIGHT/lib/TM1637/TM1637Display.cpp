#include "TM1637Display.h"
#include <Arduino.h>

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80

const uint8_t digitToSegment[] = {
  // XGFEDCBA
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01110111,    // A
  0b01111100,    // b
  0b00111001,    // C
  0b01011110,    // d
  0b01111001,    // E
  0b01110001     // F
};

TM1637Display::TM1637Display(uint8_t pinClk, uint8_t pinDIO, unsigned int bitDelay)
{
	m_pinClk = pinClk;
	m_pinDIO = pinDIO;
	m_bitDelay = bitDelay;

	pinMode(m_pinClk, INPUT);
	pinMode(m_pinDIO, INPUT);
	digitalWrite(m_pinClk, LOW);
	digitalWrite(m_pinDIO, LOW);
}

void TM1637Display::setBrightness(uint8_t brightness, bool on)
{
	brightness = (brightness & 0x7) | (on ? 0x08 : 0x00);
	start();
	writeByte(0x88 | brightness);
	stop();
}

void TM1637Display::setSegments(const uint8_t segments[], uint8_t length, uint8_t pos)
{
    if (pos > 3) return;
    if (length > 4) length = 4;
    
	start();
	writeByte(TM1637_I2C_COMM1);
	stop();

	start();
	writeByte(TM1637_I2C_COMM2 + (pos & 0x03));
	for (uint8_t k=0; k < length; k++) {
		writeByte(segments[k]);
	}
	stop();

	start();
	writeByte(TM1637_I2C_COMM3 + (0x0f & 0x07) + 0x08); // Max brightness
	stop();
}

void TM1637Display::showNumberDec(int num, bool leading_zeros, uint8_t length, uint8_t pos)
{
	showNumberDecEx(num, 0, leading_zeros, length, pos);
}

void TM1637Display::showNumberDecEx(int num, uint8_t dots, bool leading_zeros, uint8_t length, uint8_t pos)
{
    uint8_t digits[4];
	const static int divisors[] = { 1, 10, 100, 1000 };
	bool blank = false;
    
    // Xử lý số âm cơ bản
    if(num < 0) { num = -num; } 

	for(int k=0; k < 4; k++) {
	    int divisor = divisors[4 - 1 - k];
	    int d = num / divisor;
	    uint8_t digit = 0;

	    if (k >= pos && k < pos+length) {
            digit = encodeDigit(d % 10);
            if (dots & (1 << (4 - 1 - k))) {
                digit |= 0x80;
            }
            digits[k] = digit;
	    }
	}
	setSegments(digits + (4-length), length, pos);
}

void TM1637Display::clear()
{
    uint8_t data[] = { 0, 0, 0, 0 };
	setSegments(data);
}

void TM1637Display::bitDelay()
{
	delayMicroseconds(m_bitDelay);
}

void TM1637Display::start()
{
	pinMode(m_pinDIO, OUTPUT);
	bitDelay();
}

void TM1637Display::stop()
{
	pinMode(m_pinDIO, OUTPUT);
	bitDelay();
	pinMode(m_pinClk, INPUT);
	bitDelay();
	pinMode(m_pinDIO, INPUT);
	bitDelay();
}

bool TM1637Display::writeByte(uint8_t b)
{
	uint8_t data = b;

	// 8 Data Bits
	for(uint8_t i = 0; i < 8; i++) {
		pinMode(m_pinClk, OUTPUT);
		bitDelay();
		if(data & 0x01)
			pinMode(m_pinDIO, INPUT);
		else
			pinMode(m_pinDIO, OUTPUT);
		bitDelay();
		pinMode(m_pinClk, INPUT);
		bitDelay();
		data = data >> 1;
	}

	// Wait for ACK
	pinMode(m_pinClk, OUTPUT);
	pinMode(m_pinDIO, INPUT);
	bitDelay();
	pinMode(m_pinClk, INPUT);
	bitDelay();
	uint8_t ack = digitalRead(m_pinDIO);
	if (ack == 0)
		pinMode(m_pinDIO, OUTPUT);
	bitDelay();
	pinMode(m_pinClk, OUTPUT);
	bitDelay();

	return ack;
}

uint8_t TM1637Display::encodeDigit(uint8_t digit)
{
	return digitToSegment[digit & 0x0f];
}