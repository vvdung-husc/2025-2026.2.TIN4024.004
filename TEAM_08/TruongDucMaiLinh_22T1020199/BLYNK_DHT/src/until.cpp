#include "utils.h"

#define DAY_THRESHOLD 2000

// BUTTON
void BUTTON::begin(int pin)
{
  _pin = pin;
  pinMode(_pin, INPUT);
}

void BUTTON::update()
{
  _state = digitalRead(_pin);
}

bool BUTTON::isPressed()
{
  return _state == HIGH;
}

// LED
void LED::begin(int pin)
{
  _pin = pin;
  pinMode(_pin, OUTPUT);
}

void LED::set(bool on)
{
  digitalWrite(_pin, on);
}

// LDR
void LDR::begin(int pin)
{
  _pin = pin;
}

int LDR::read()
{
  return analogRead(_pin);
}

bool LDR::isDark()
{
  return read() > DAY_THRESHOLD;
}

// TRAFFIC
void Traffic::begin(int red,int yellow,int green)
{
  leds[RED].begin(red);
  leds[YELLOW].begin(yellow);
  leds[GREEN].begin(green);
}

void Traffic::setTime(int red,int yellow,int green)
{
  waitTime[RED] = red*1000;
  waitTime[YELLOW] = yellow*1000;
  waitTime[GREEN] = green*1000;

  idx = GREEN;
  count = waitTime[idx];
}

void Traffic::run(LDR &ldr)
{
  if(millis()-timer < 500) return;
  timer = millis();

  if(ldr.isDark())
  {
    nightMode = true;

    leds[GREEN].set(0);
    leds[RED].set(0);

    ledState = !ledState;
    leds[YELLOW].set(ledState);

    return;
  }

  if(nightMode)
  {
    nightMode = false;
    idx = GREEN;
    count = waitTime[idx];
  }

  if(count == waitTime[idx])
  {
    secondCount = (count/1000)-1;

    for(int i=0;i<3;i++)
      leds[i].set(i==idx);
  }
  else
  {
    ledState = !ledState;
    leds[idx].set(ledState);
  }

  if(ledState)
    secondCount--;

  count -= 500;

  if(count <= 0)
  {
    idx = (idx+1)%3;
    count = waitTime[idx];
  }
}

int Traffic::getCount()
{
  return secondCount+1;
}

bool Traffic::isNight()
{
  return nightMode;
}