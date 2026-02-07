#ifndef ULTILS_H
#define ULTILS_H

#include <Arduino.h>
#include <Wire.h>

// OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// DHT
#include <DHT.h>

// ================= MyLED =================
class MyLED {
private:
    int _pin;
    bool _ledState;
    unsigned long _timer;

public:
    MyLED();
    void setup(int pin);
    void blink(uint32_t interval);
    void on();
    void off();
};

// ================= MyDHT =================
class MyDHT {
private:
    int _pin;
    int _type;
    DHT* _dht;
    float _temp;
    float _hum;
    unsigned long _timer;

public:
    MyDHT();
    void setup(int pin, int type);
    void run();
    float getTemp();
    float getHum();
};

// ================= MyOLED =================
class MyOLED {
private:
    Adafruit_SSD1306* _display;
    uint8_t _address;

public:
    MyOLED();
    void setup(int sda, int scl, uint8_t address);
    void displayInfo(float temp, float hum, String statusMsg);
};

// ================= Controller =================
class SystemController {
private:
    unsigned long _timer;

public:
    SystemController();
    void run(MyLED &ledGreen,
             MyLED &ledYellow,
             MyLED &ledRed,
             MyDHT &dht,
             MyOLED &oled);
};

#endif
