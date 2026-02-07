#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Hàm tiện ích xử lý thời gian (Non-blocking)
bool IsReady(unsigned long &ulTimer, uint32_t millisecond);

// --- Class LED ---
class MyLED {
public:
    MyLED();
    void setup(int pin);
    void blink(uint32_t interval);
    void off();
    void on();
private:
    int _pin;
    bool _ledState;
    unsigned long _timer;
};

// --- Class DHT Sensor ---
class MyDHT {
public:
    MyDHT();
    void setup(int pin, int type);
    void run();
    float getTemp();
    float getHum();
private:
    DHT* _dht;
    int _pin;
    int _type;
    float _temp;
    float _hum;
    unsigned long _timer;
};

// --- Class OLED Display ---
class MyOLED {
public:
    MyOLED();
    void setup(int sda, int scl, uint8_t address);
    // Hàm hiển thị được cập nhật layout mới
    void displayInfo(float temp, float hum, String statusMsg);
private:
    Adafruit_SSD1306* _display;
    uint8_t _address;
};

// --- Class System Controller ---
class SystemController {
public:
    SystemController();
    void run(MyLED &ledGreen, MyLED &ledYellow, MyLED &ledRed, MyDHT &dht, MyOLED &oled);
private:
    unsigned long _timer;
};