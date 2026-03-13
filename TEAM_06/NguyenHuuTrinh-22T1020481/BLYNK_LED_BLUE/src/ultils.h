#pragma once
#include <Arduino.h>
#include <DHT.h>
#include <TM1637Display.h>

// Hàm tiện ích xử lý thời gian (Non-blocking)
bool IsReady(unsigned long &ulTimer, uint32_t millisecond);

// --- Class LED ---
class MyLED {
public:
    MyLED();
    void setup(int pin);
    void set(bool state);
    bool getState();
private:
    int _pin;
    bool _state;
};

// --- Class Nút nhấn (Button) ---
class MyButton {
public:
    MyButton();
    void setup(int pin);
    bool isPressed();
private:
    int _pin;
    bool _lastState;
    unsigned long _debounceTimer;
};

// --- Class Cảm biến DHT ---
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

// --- Class Màn hình TM1637 ---
class MyDisplay {
public:
    MyDisplay();
    void setup(int clk, int dio);
    void showNumber(int num);
    void clear();
private:
    TM1637Display* _tm;
};

// --- Class Quản lý Hệ thống ---
class SystemController {
public:
    SystemController();
    void run(MyLED &led, MyButton &btn, MyDHT &dht, MyDisplay &display);
    void setLedState(bool state, MyLED &led); // Dùng để Blynk gọi khi ấn nút trên app
    
private:
    unsigned long _timer1s;
    unsigned long _uptime;
    bool _isLedOn;
};