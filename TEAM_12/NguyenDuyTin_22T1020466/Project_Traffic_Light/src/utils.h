#pragma once
#include <Arduino.h>
#include <TM1637Display.h>
// Định nghĩa các lớp tiện ích: BUTTON, LED, Traffic_Blink

enum LED_INDEX // Chỉ số đèn giao thông
{
    INDEX_LED_GREEN = 0,
    INDEX_LED_YELLOW,
    INDEX_LED_RED
};

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

class LDR
{
public:
    LDR();
    void setup(int pin, bool vcc5Volt = false);
    int getValue();
    float readLux(int* analogValue = nullptr);

    static int DAY_THRESHOLD; // lux

private:
    int _pin;
    int _value;
    bool _vcc5Volt;
};

class Traffic_Blink
{
public:
    Traffic_Blink();
    ~Traffic_Blink();
    void setup_Pin(int pinRed, int pinYellow, int pinGreen);
    void setup_WaitTime(int redTimer = 5, int yellowTimer = 3, int greenTimer = 7); // seconds
    //void blink(bool showLogger = false);
    void run(LDR& ldrSensor, bool showLogger = false);
    int getCount();
    bool isNightMode(); // che do dem

protected:
    LED _leds[3];
    int _waitTime[3];
    int _idxLED;
    int _secondCount;
    bool _nightMode; // che do dem
   
};

// Hàm kiểm tra thời gian đã trôi qua - Non-Blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond = 500);
// Định dạng chuỗi %s,%d,...
String StringFormat(const char *fmt, ...);