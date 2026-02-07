#include "ultils.h"
#include <math.h>

// ----- LED -----
LED::LED()
{
    _pin = -1;
    _state = false;
    _previousMillis = 0;
}

void LED::setup(int pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void LED::blink(unsigned long interval)
{
    if (!IsReady(_previousMillis, interval))
        return;
    _state = !_state;
    digitalWrite(_pin, _state ? HIGH : LOW);
}

void LED::set(bool bON)
{
    digitalWrite(_pin, bON ? HIGH : LOW);
}

// ----- Trafic_Blink -----
Trafic_Blink::Trafic_Blink()
{
    _ledStatus = false;
    _previousMillis = 0;
    _idxLED = 0;
    _secondCount = 0;
}

void Trafic_Blink::setupPin(int pinRed, int pinYellow, int pinGreen, int pinBlue, int pinButton)
{
    _LEDs[0] = pinGreen;
    _LEDs[1] = pinYellow;
    _LEDs[2] = pinRed;

    pinMode(pinRed, OUTPUT);
    pinMode(pinYellow, OUTPUT);
    pinMode(pinGreen, OUTPUT);

    digitalWrite(pinRed, LOW);
    digitalWrite(pinYellow, LOW);
    digitalWrite(pinGreen, LOW);

    _idxLED = 0;
    _secondCount = 0;

    ledBlue.setup(pinBlue);

    _pinButton = pinButton;
    // mạch bạn: nút nối về GND -> ACTIVE LOW
    pinMode(_pinButton, INPUT_PULLUP);
}

void Trafic_Blink::setupWaitTime(uint32_t redWait, uint32_t yellowWait, uint32_t greenWait)
{
    _waitTime[0] = greenWait * 1000UL;
    _waitTime[1] = yellowWait * 1000UL;
    _waitTime[2] = redWait * 1000UL;
    _secondCount = 0;
}

void Trafic_Blink::run(LDR &ldrSensor, TM1637Display &display, bool showLogger)
{
    static uint32_t count = 0;
    static bool prevDark = false;
    static bool prevButton = false;
    static bool displayEnabled = false;

    // đọc nút
    bool bButtonON = isButtonON();
    if (bButtonON && !prevButton)
    {
        displayEnabled = !displayEnabled;
        if (!displayEnabled)
            display.clear();
        ledBlue.set(displayEnabled);
    }
    prevButton = bButtonON;

    // tick 500ms
    if (!IsReady(_previousMillis, 500))
        return;

    // đọc LDR (timer 200ms)
    int ldrValue = ldrSensor.getValue(200);
    bool isDark = (ldrValue > ldrSensor.DAY_THRESHOLD);

    // BAN ĐÊM: chỉ nhấp nháy vàng
    if (isDark)
    {
        if (prevDark != isDark)
        {
            prevDark = isDark;
            digitalWrite(_LEDs[0], LOW); // GREEN OFF
            digitalWrite(_LEDs[2], LOW); // RED OFF
            display.clear();
            if (showLogger)
                printf("IT IS DARK! LDR=%d\n", ldrValue);
        }

        _ledStatus = !_ledStatus;
        digitalWrite(_LEDs[1], _ledStatus ? HIGH : LOW);
        return;
    }

    // chuyển từ đêm -> ngày
    if (prevDark != isDark && prevDark == true)
    {
        if (showLogger)
            printf("IT IS DAY! LDR=%d\n", ldrValue);
        prevDark = isDark;
        _ledStatus = false;
        _idxLED = 0;
        count = 0;
    }

    // BAN NGÀY: chạy 3 màu theo thời gian
    if (count == 0)
    {
        count = _waitTime[_idxLED];
    }

    if (count == _waitTime[_idxLED])
    {
        _secondCount = (count / 1000) - 1;

        _ledStatus = true;
        for (int i = 0; i < 3; i++)
        {
            digitalWrite(_LEDs[i], (i == _idxLED) ? HIGH : LOW);
        }

        if (showLogger)
        {
            printf("LED [%s] ON => %lu Seconds\n", ledString(_LEDs[_idxLED]), count / 1000);
        }
    }
    else
    {
        // nhấp nháy LED hiện tại mỗi 500ms (giống thầy)
        _ledStatus = !_ledStatus;
        digitalWrite(_LEDs[_idxLED], _ledStatus ? HIGH : LOW);
    }

    if (_ledStatus)
    {
        if (displayEnabled)
            display.showNumberDec(_secondCount);
        if (showLogger)
            printf(" [%s] => seconds: %d\n", ledString(_LEDs[_idxLED]), _secondCount);
        --_secondCount;
    }

    // giảm 500ms
    if (count >= 500)
        count -= 500;
    if (count > 0)
        return;

    // hết thời gian -> chuyển LED
    _idxLED = (_idxLED + 1) % 3;
    count = _waitTime[_idxLED];
    _secondCount = 0;
}

bool Trafic_Blink::isButtonON()
{
    static unsigned long ulTimerButton = 0;
    static bool btnStatus = false;

    if (!IsReady(ulTimerButton, 10))
        return btnStatus;

    // mạch bạn: bấm -> LOW
    btnStatus = (digitalRead(_pinButton) == LOW);
    return btnStatus;
}

const char *Trafic_Blink::ledString(int pin)
{
    if (pin == _LEDs[2])
        return "RED";
    if (pin == _LEDs[1])
        return "YELLOW";
    if (pin == _LEDs[0])
        return "GREEN";
    return "UNKNOWN";
}

// ----- LDR -----
int LDR::DAY_THRESHOLD = 2000;

LDR::LDR()
{
    _pin = -1;
    _value = 0;
    _vcc5Volt = true;
}

void LDR::setup(int pin, bool vcc5Volt)
{
    _pin = pin;
    _vcc5Volt = vcc5Volt;
    pinMode(_pin, INPUT);
}

int LDR::getValue(int timer)
{
    static unsigned long ulTimer_getValue = 0;
    if (timer > 0 && !IsReady(ulTimer_getValue, timer))
        return _value;
    _value = analogRead(_pin);
    return _value;
}

float LDR::readLux(int *analogValue)
{
    float voltage, resistance, lux;

    getValue(100);
    if (analogValue)
        *analogValue = _value;

    // ESP32 ADC: 0..4095
    if (_vcc5Volt)
    {
        voltage = (float)_value * 5.0f / 4095.0f;
        resistance = 2000.0f * voltage / (1.0f - voltage / 5.0f);
        lux = pow(50e3f * pow(10.0f, 0.7f) / resistance, (1.0f / 0.7f));
    }
    return lux;
}
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
    if (millis() - ulTimer < millisecond)
        return false;
    ulTimer = millis();
    return true;
}
