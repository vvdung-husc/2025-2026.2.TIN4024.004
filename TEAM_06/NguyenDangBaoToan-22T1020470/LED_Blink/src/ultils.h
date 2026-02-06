#pragma once
#include <Arduino.h>

bool IsReady(unsigned long &ulTimer, uint32_t millisecond);

class LED {
public:
  LED();
  void setup(int pin);
  void on();
  void off();
  void toggle();
  void blink(unsigned long interval = 500);

private:
  int _pin;
  bool _state;
  unsigned long _previousMillis;
};

class Trafic_Blink {
public:
  Trafic_Blink();
  void setupPin(int pinRed, int pinYellow, int pinGreen);
  void setupWaitTime(uint32_t redWait = 1, uint32_t yellowWait = 1, uint32_t greenWait = 1); // seconds
  void run(); 

private:
  bool _ledStatus;
  unsigned long _previousMillis;
  int _LEDs[3];        // [0]=GREEN, [1]=YELLOW, [2]=RED
  int _idxLED;
  uint32_t _waitTime[3]; // ms
};
