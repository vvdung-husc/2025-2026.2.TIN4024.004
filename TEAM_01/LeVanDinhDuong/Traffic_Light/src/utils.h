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
    void setup_WaitTime(int redTimer = 5, int yellowTimer = 3, int greenTimer = 7); 
    void blink(bool showLogger = false);
    int getCount();

protected:
    LED _leds[3];
    int _waitTime[3];
    int _idxLED;
    int _secondCount;
};

class SENSOR_LDR
{
public:
    SENSOR_LDR();
    ~SENSOR_LDR();
    void setup(int pin);
    int getLux();          
    bool isDark(int limit = 2000); 

protected:
    int _pin;
};

bool IsReady(unsigned long &ulTimer, uint32_t millisecond = 500);
String StringFormat(const char *fmt, ...);