#include <Arduino.h>
#include <TM1637Display.h>
#include <stdarg.h>
#include <math.h>

// ===================== PIN DEFINITIONS =====================
#define PIN_LED_RED       18
#define PIN_LED_YELLOW     5
#define PIN_LED_GREEN     17

#define PIN_LDR           34   // ADC GPIO34
#define PIN_CLK           22
#define PIN_DIO           23

#define PIN_LED_BLUE      12
#define PIN_BUTTON_BLUE   13

// ===================== UTILS =====================
bool IsReady(unsigned long &ulTimer, uint32_t millisecond = 1000)
{
    if (millis() - ulTimer < millisecond)
        return false;
    ulTimer = millis();
    return true;
}

String StringFormat(const char *fmt, ...)
{
    va_list vaArgs;
    va_start(vaArgs, fmt);
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, vaArgs);

    int len = vsnprintf(NULL, 0, fmt, vaArgsCopy);
    va_end(vaArgsCopy);

    char *buff = (char *)malloc(len + 1);
    vsnprintf(buff, len + 1, fmt, vaArgs);
    va_end(vaArgs);

    String s = buff;
    free(buff);
    return s;
}

// ===================== LED CLASS =====================
class LED
{
public:
    LED() : _pin(-1), _state(false), _previousMillis(0) {}

    void setup(int pin)
    {
        _pin = pin;
        pinMode(_pin, OUTPUT);
    }

    void blink(unsigned long interval = 500)
    {
        if (!IsReady(_previousMillis, interval)) return;
        _state = !_state;
        digitalWrite(_pin, _state ? HIGH : LOW);
    }

    void set(bool on)
    {
        digitalWrite(_pin, on ? HIGH : LOW);
    }

private:
    int _pin;
    bool _state;
    unsigned long _previousMillis;
};

// ===================== LDR CLASS =====================
class LDR
{
public:
    static int DAY_THRESHOLD;

    LDR() : _pin(-1), _value(0), _vcc5Volt(true) {}

    void setup(int pin, bool vcc5Volt = true)
    {
        _pin = pin;
        _vcc5Volt = vcc5Volt;
        pinMode(_pin, INPUT);
    }

    int getValue()
    {
        _value = analogRead(_pin);
        return _value;
    }

    float readLux(int *analogValue = nullptr)
    {
        float voltage, resistance, lux;

        getValue();
        if (analogValue) *analogValue = _value;

        if (_vcc5Volt)
        {
            voltage = (float)_value * 5.0 / 4095.0;
            resistance = 2000 * voltage / (1 - voltage / 5.0);
            lux = pow(50e3 * pow(10, 0.7) / resistance, (1 / 0.7));
        }
        else
        {
            voltage = (float)_value * 3.3 / 4095.0;
            resistance = 2000 * voltage / (1 - voltage / 3.3);
            lux = pow(33e3 * pow(10, 0.7) / resistance, (1 / 0.7));
        }
        return lux;
    }

private:
    int _pin;
    int _value;
    bool _vcc5Volt;
};

int LDR::DAY_THRESHOLD = 2000;

// ===================== TRAFFIC LIGHT CLASS =====================
class Trafic_Blink
{
public:
    Trafic_Blink() : _ledStatus(false), _previousMillis(0),
                     _idxLED(0), _secondCount(0), _pinButton(-1) {}

    void setupPin(int pinRed, int pinYellow, int pinGreen, int pinBlue, int pinButton)
    {
        _LEDs[0] = pinGreen;
        _LEDs[1] = pinYellow;
        _LEDs[2] = pinRed;

        for (int i = 0; i < 3; i++)
            pinMode(_LEDs[i], OUTPUT);

        ledBlue.setup(pinBlue);
        _pinButton = pinButton;
        pinMode(_pinButton, INPUT);
    }

    void setupWaitTime(uint32_t redWait, uint32_t yellowWait, uint32_t greenWait)
    {
        _waitTime[0] = greenWait * 1000;
        _waitTime[1] = yellowWait * 1000;
        _waitTime[2] = redWait * 1000;
    }

    void blink(LDR &ldr, TM1637Display &display, bool showLogger = true)
    {
        static uint32_t count = _waitTime[_idxLED];
        static bool prevDark = false;
        static bool prevButton = false;

        if (!IsReady(_previousMillis, 500)) return;

        bool btn = isButtonON();
        if (btn != prevButton)
        {
            ledBlue.set(btn);
            if (!btn) display.clear();
            prevButton = btn;
        }

        bool isDark = (ldr.getValue() > LDR::DAY_THRESHOLD);

        if (isDark)
        {
            if (!prevDark)
            {
                digitalWrite(_LEDs[0], LOW);
                digitalWrite(_LEDs[2], LOW);
                display.clear();
                prevDark = true;
            }
            _ledStatus = !_ledStatus;
            digitalWrite(_LEDs[1], _ledStatus ? HIGH : LOW);
            return;
        }

        if (prevDark)
        {
            prevDark = false;
            _idxLED = 0;
            count = _waitTime[_idxLED];
        }

        if (count == _waitTime[_idxLED])
        {
            _secondCount = count / 1000 - 1;
            for (int i = 0; i < 3; i++)
                digitalWrite(_LEDs[i], i == _idxLED ? HIGH : LOW);
        }

        if (_ledStatus && btn)
            display.showNumberDec(_secondCount);

        _ledStatus = !_ledStatus;
        digitalWrite(_LEDs[_idxLED], _ledStatus ? HIGH : LOW);
        if (_ledStatus) _secondCount--;

        count -= 500;
        if (count <= 0)
        {
            _idxLED = (_idxLED + 1) % 3;
            count = _waitTime[_idxLED];
        }
    }

    bool isButtonON()
    {
        static unsigned long t = 0;
        static bool state = false;
        if (!IsReady(t, 10)) return state;
        state = digitalRead(_pinButton) == HIGH;
        return state;
    }

private:
    bool _ledStatus;
    unsigned long _previousMillis;
    int _LEDs[3];
    int _idxLED;
    uint32_t _waitTime[3];
    int _secondCount;
    int _pinButton;
    LED ledBlue;
};

// ===================== GLOBAL OBJECTS =====================
Trafic_Blink traficLight;
LDR ldrSensor;
TM1637Display display(PIN_CLK, PIN_DIO);

// ===================== SETUP & LOOP =====================
void setup()
{
    Serial.begin(115200);
    printf("Welcome IoT\n");

    ldrSensor.setup(PIN_LDR, false); // 3.3V
    traficLight.setupPin(
        PIN_LED_RED,
        PIN_LED_YELLOW,
        PIN_LED_GREEN,
        PIN_LED_BLUE,
        PIN_BUTTON_BLUE
    );

    traficLight.setupWaitTime(5, 3, 7);
    display.setBrightness(0x0A);
    display.clear();
}

void loop()
{
    traficLight.blink(ldrSensor, display);
}
