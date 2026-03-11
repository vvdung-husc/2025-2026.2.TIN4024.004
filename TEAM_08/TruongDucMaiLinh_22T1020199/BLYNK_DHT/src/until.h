#pragma once
#include <Arduino.h>

enum LED_INDEX
{
  GREEN = 0,
  YELLOW,
  RED
};

class BUTTON
{
public:
  void begin(int pin);
  void update();
  bool isPressed();

private:
  int _pin;
  bool _state;
};

class LED
{
public:
  void begin(int pin);
  void set(bool on);

private:
  int _pin;
};

class LDR
{
public:
  void begin(int pin);
  int read();
  bool isDark();

private:
  int _pin;
};

class Traffic
{
public:
  void begin(int red,int yellow,int green);
  void setTime(int red,int yellow,int green);
  void run(LDR &ldr);

  int getCount();
  bool isNight();

private:
  LED leds[3];

  int waitTime[3];
  int idx = GREEN;

  int secondCount = 0;

  bool nightMode = false;

  unsigned long timer = 0;
  int count = 0;
  bool ledState = false;
};