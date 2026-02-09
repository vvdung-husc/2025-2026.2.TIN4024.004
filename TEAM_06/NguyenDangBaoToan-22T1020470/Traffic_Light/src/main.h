#pragma once
#include <Arduino.h>
#include "ultils.h"

// Traffic LEDs (theo mạch bạn)
#define PIN_LED_RED     26
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

// TM1637
#define PIN_CLK  18
#define PIN_DIO  19

// Blue LED + Button
#define PIN_LED_BLUE      21
#define PIN_BUTTON_BLUE   23   // nút nối về GND => ACTIVE LOW

// LDR
#define PIN_LDR  13            // AO -> GPIO13
