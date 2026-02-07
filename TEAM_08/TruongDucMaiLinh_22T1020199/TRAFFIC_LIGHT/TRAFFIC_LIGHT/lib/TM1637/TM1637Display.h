#ifndef __TM1637DISPLAY__
#define __TM1637DISPLAY__

#include <inttypes.h>
#include <Arduino.h>

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80

class TM1637Display {
public:
  TM1637Display(uint8_t pinClk, uint8_t pinDIO, unsigned int bitDelay = 100);
  void setBrightness(uint8_t brightness, bool on = true);
  void setSegments(const uint8_t segments[], uint8_t length = 4, uint8_t pos = 0);
  void showNumberDec(int num, bool leading_zeros = false, uint8_t length = 4, uint8_t pos = 0);
  void showNumberDecEx(int num, uint8_t dots = 0, bool leading_zeros = false, uint8_t length = 4, uint8_t pos = 0);
  void clear();

private:
  uint8_t m_pinClk;
  uint8_t m_pinDIO;
  unsigned int m_bitDelay;

  void bitDelay();
  void start();
  void stop();
  bool writeByte(uint8_t b);
  uint8_t encodeDigit(uint8_t digit);
};

#endif