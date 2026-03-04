#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>

// ===== LED CLASS =====
class LED {
private:
  int _pin;
  int _state;                 // 0: ON, 1: HOLD, 2: OFF
  unsigned long _timer;

public:
  LED();
  void setup(int pin);
  void off();
  void blink(uint32_t onTime, uint32_t holdTime, uint32_t offTime);
};

// ===== SYSTEM CLASS =====
class DHT_OLED {
private:
  DHT _dht;

  float _temp;
  float _humi;
  String _status;
  int _activeLED;

  LED _green, _yellow, _red;
  unsigned long _sensorTimer;

  void readSensor();
  void updateStatus();
  void blinkLED();

public:
  DHT_OLED();
  void setup(int pinDHT, int ledGreen, int ledYellow, int ledRed);
  void run(Adafruit_SSD1306& display);
  void showOLED(Adafruit_SSD1306& display);
};

#endif