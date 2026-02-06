
#pragma once
#include <Arduino.h>
#include <TM1637Display.h>

class BUTTON
{
public:
    BUTTON();
    ~BUTTON();
    void setup(int pin);
    void processPressed();
    bool isPressed();
protected:
    int _pin;
    int _prevValue;
};

class LED
{
public:
    LED();
    virtual ~LED();
    const char *getName();
    void setup(int pin, const char *name);
    void setStatus(bool bON);
    void blink();

protected:
    int _pin;
    bool _status;
    String _name;
};

class Traffic_Blink
{
public:
    Traffic_Blink();
    ~Traffic_Blink();
    void setup_Pin(int pinRed, int pinYellow, int pinGreen);
    void setup_WaitTime(int redTimer = 5, int yellowTimer = 3, int greenTimer = 7); // seconds
    void blink(bool showLogger = false);
    int getCount();

protected:
    LED _leds[3];
    int _waitTime[3];
    int _idxLED;
    int _secondCount;
};

// Hàm kiểm tra thời gian đã trôi qua - Non-Blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond = 500);
// Định dạng chuỗi %s,%d,...
String StringFormat(const char *fmt, ...);
