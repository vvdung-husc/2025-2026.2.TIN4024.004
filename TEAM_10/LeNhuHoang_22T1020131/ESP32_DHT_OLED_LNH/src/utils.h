#pragma once
#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class LED {
public:
    LED();
    void setup(int pin);
    bool blink(unsigned long interval = 500);
    void off();
private:
    int _pin;
    bool _state;
    unsigned long _timer;
};

class DHT_OLED {
public:
    DHT_OLED();
    void setup(int pinDHT, int ledGreen, int ledYellow, int ledRed);
    void run(Adafruit_SSD1306& display);

private:
    void showOLED(Adafruit_SSD1306& display);
    void updateLED();
    void blinkLED();

    DHT _dht;

    LED _ledGreen;
    LED _ledYellow;
    LED _ledRed;

    float _temperature;
    float _humidity;
    const char* _status;

    int _blinkLED;
};

bool IsReady(unsigned long& timer, uint32_t ms = 1000);
