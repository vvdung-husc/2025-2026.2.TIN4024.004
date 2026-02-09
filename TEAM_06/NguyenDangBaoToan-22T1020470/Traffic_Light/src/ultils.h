#pragma once
#include <Arduino.h>
#include <TM1637Display.h>

bool IsReady(unsigned long &ulTimer, uint32_t millisecond = 1000);

class LED {
public:
  LED();
  void setup(int pin);
  void blink(unsigned long interval = 500);
  void set(bool bON);

private:
  int _pin;
  bool _state;
  unsigned long _previousMillis;
};

class LDR {
public:
  static int DAY_THRESHOLD;

public:
  LDR();
  void setup(int pin, bool vcc5Volt = true);
  int getValue(int timer = 0);                 // analog 0..4095
  float readLux(int* analogValue = nullptr);

private:
  int _pin;
  int _value;
  bool _vcc5Volt;
};

class Trafic_Blink {
public:
  Trafic_Blink();
  void setupPin(int pinRed, int pinYellow, int pinGreen, int pinBlue, int pinButton);
  void setupWaitTime(uint32_t redWait = 5, uint32_t yellowWait = 3, uint32_t greenWait = 10);
  void run(LDR& ldrSensor, TM1637Display& display, bool showLogger = true);

private:
  const char *ledString(int pin);
  bool isButtonON();  // đã sửa theo mạch bạn (ACTIVE LOW)

private:
  bool _ledStatus;
  unsigned long _previousMillis;
  int _LEDs[3];         // [0]=GREEN, [1]=YELLOW, [2]=RED
  int _idxLED;
  uint32_t _waitTime[3];
  int _secondCount;

  int _pinButton;
  LED ledBlue;
};
