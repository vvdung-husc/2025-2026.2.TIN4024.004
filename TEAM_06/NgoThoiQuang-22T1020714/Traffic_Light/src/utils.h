#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <TM1637Display.h>

extern bool extraMode;

void handleButton();
void showCountdown(const char* color, int seconds);
void turnOffAllTrafficLED();
float readLux();

#endif
