#include "utils.h"

//#define INDEX_LED_GREEN 0 
//#define INDEX_LED_YELLOW 1
//#define INDEX_LED_RED 2

//----- class BUTTON --------------------
BUTTON::BUTTON()
{
    _pin = -1;
    _prevValue = LOW;
}
BUTTON::~BUTTON()
{
}
void BUTTON::setup(int pin)
{
    _pin = pin;
    _prevValue = LOW;
    pinMode(_pin, INPUT);
}
void BUTTON::processPressed()
{
    static ulong ulTimer = 0;

    if (!IsReady(ulTimer, 10))
        return;

    int newValue = digitalRead(_pin);
    if (newValue == _prevValue)
        return;
    _prevValue = newValue;
}

bool BUTTON::isPressed()
{
    return (_prevValue == HIGH);
}

//----- class LED --------------------
LED::LED()
{
    _pin = -1;
    _name = "UNKNOW";
    _status = false;
}
LED::~LED() {}

const char *LED::getName()
{
    return _name.c_str();
}

void LED::setup(int pin, const char *name)
{
    _pin = pin;
    if (name && name[0] != 0)
        _name = name;

    _status = false;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void LED::setStatus(bool bON)
{
    digitalWrite(_pin, bON ? HIGH : LOW);
}

void LED::blink()
{
    static unsigned long ulTimer = 0;
    if (!IsReady(ulTimer, 500))
        return;
    _status = !_status;
    digitalWrite(_pin, _status ? HIGH : LOW);
}



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

int LDR::getValue()
{
    _value = analogRead(_pin);
    return _value;
}
void Traffic_Blink::run(LDR& ldrSensor, bool showLogger) // dieu khien den giao thong
{
    static unsigned long ulTimer = 0;
    static uint32_t count = _waitTime[_idxLED];
    static bool ledStatus = false;

    // chạy mỗi 500ms
    if (!IsReady(ulTimer, 500))
        return;

    // ---- ĐỌC LDR ----
    int adcValue;
    ldrSensor.readLux(&adcValue);
    bool isDark = (adcValue > LDR::DAY_THRESHOLD);

    // ================== NIGHT MODE ==================
    if (isDark)
    {
        if (!_nightMode)
        {
            _nightMode = true;
            
            // Tắt xanh & đỏ
            _leds[INDEX_LED_GREEN].setStatus(false);
            _leds[INDEX_LED_RED].setStatus(false);

            if (showLogger)
                printf(" NIGHT MODE - YELLOW BLINKING\n");
        }

        // Nhấp nháy vàng
        ledStatus = !ledStatus;
        _leds[INDEX_LED_YELLOW].setStatus(ledStatus);
        return;
    }

    // ================== BACK TO DAY ==================
    if (_nightMode && !isDark)
    {
        _nightMode = false; // thoat che do dem

         // Tat den vang
        ledStatus = false;

        _idxLED = INDEX_LED_GREEN; // bat dau tu den xanh
        count = _waitTime[_idxLED];
        _secondCount = 0;

        if (showLogger)
            printf(" DAY MODE - TRAFFIC NORMAL\n");
    }

    // ================== NORMAL TRAFFIC ==================
    if (count == _waitTime[_idxLED]) //
    {
        _secondCount = (count / 1000) - 1;
        ledStatus = true;
        // Bat den hien tai, tat cac den con lai
        for (int i = 0; i < 3; i++)
        {
            if (i == _idxLED)
            {
                _leds[i].setStatus(true);
                if (showLogger)
                    printf("LED [%-6s] ON => %d Seconds\n",
                           _leds[i].getName(), count / 1000);
            }
            else
                _leds[i].setStatus(false);
        }
    }
    else
    {
        ledStatus = !ledStatus;
        _leds[_idxLED].setStatus(ledStatus);
    }

    if (ledStatus) 
    {
        if (showLogger)
            printf(" [%s] => seconds: %d\n",
                   _leds[_idxLED].getName(), _secondCount);
        --_secondCount;
    }

    count -= 500;
    if (count > 0)
        return;

    _idxLED = (_idxLED + 1) % 3;
    count = _waitTime[_idxLED];
}
//----- class LDR --------------------
float LDR::readLux(int *analogValue)
{
    static float prevLux = -1.0;
    float voltage, resistance, lux;

    getValue();// doc gia tri ADC

    if (analogValue != nullptr)
    {
        *analogValue = _value;
    }

    if (_vcc5Volt)
    {
        // VCC = 5V
        voltage = (float)_value / 4096.0 * 5.0;
        resistance = 2000 * voltage / (1 - voltage / 5.0);
        lux = pow(50 * 1e3 * pow(10, 0.7) / resistance, (1 / 0.7));
        return lux;
    }
    else
    {
        // VCC = 3.3V
        voltage = (float)_value / 4096.0 * 3.3 ;
        resistance = 2000 * voltage / (1 - voltage / 3.3);
        lux = pow(33 * 1e3 * pow(10, 0.7) / resistance, (1 / 0.7));
    }

    if (lux != prevLux)
    {
        prevLux = lux;
        if (prevLux >= 1.0)
            printf("LDR Analog: %d, Voltage: %.2f V, Resistance: %.2f Ohm, Light Intensity: %.2f[%.0f] lux\n", _value, voltage, resistance, lux, prevLux);
        else
            printf("LDR Analog: %d, Voltage: %.2f V, Resistance: %.2f Ohm, Light Intensity: %.2f[%.1f] lux\n", _value, voltage, resistance, lux, prevLux);
    }

    return lux;
}
//----- class Traffic_Blink --------------------

bool Traffic_Blink::isNightMode()
{
    return _nightMode;
}

Traffic_Blink::Traffic_Blink()
{
    _idxLED = INDEX_LED_GREEN;
    _waitTime[INDEX_LED_GREEN] = 7 * 1000;
    _waitTime[INDEX_LED_YELLOW] = 3 * 1000;
    _waitTime[INDEX_LED_RED] = 5 * 1000;
    _secondCount = 0;
    _nightMode = false;

}

Traffic_Blink::~Traffic_Blink()
{
}
// thiet lap chan cho tung den
void Traffic_Blink::setup_Pin(int pinRed, int pinYellow, int pinGreen)
{
    _leds[INDEX_LED_GREEN].setup(pinGreen, "GREEN");
    _leds[INDEX_LED_YELLOW].setup(pinYellow, "YELLOW");
    _leds[INDEX_LED_RED].setup(pinRed, "RED");
}
// thiet lap thoi gian cho tung den
void Traffic_Blink::setup_WaitTime(int redTimer, int yellowTimer, int greenTimer)
{
    _idxLED = INDEX_LED_GREEN;
    _waitTime[INDEX_LED_GREEN] = greenTimer * 1000;
    _waitTime[INDEX_LED_YELLOW] = yellowTimer * 1000;
    _waitTime[INDEX_LED_RED] = redTimer * 1000;
    _secondCount = 0;
}
// dieu khien den giao thong
/* void Traffic_Blink::blink(bool showLogger)
{
    static unsigned long ulTimer = 0;
    static uint32_t count = _waitTime[_idxLED];
    static bool ledStatus = false;

    if (!IsReady(ulTimer, 500))
        return;
// dieu khien den giao thong
    if (count == _waitTime[_idxLED])
    {
        _secondCount = (count / 1000) - 1;

        ledStatus = true;
        for (size_t i = 0; i < 3; i++)
        {
            if (i == _idxLED)
            {
                _leds[i].setStatus(true);
                if (showLogger)
                    printf("LED [%-6s] ON => %d Seconds\n", _leds[i].getName(), count / 1000);
            }
            else
                _leds[i].setStatus(false);
        }
    }
    else
    {
        ledStatus = !ledStatus;
        _leds[_idxLED].setStatus(ledStatus);
    }
// thoi gian dem nguoc
    if (ledStatus)
    {
        if (showLogger)
            printf(" [%s] => seconds: %d \n", _leds[_idxLED].getName(), _secondCount);
        --_secondCount;
    }

    count -= 500;
    if (count > 0)
        return;

    _idxLED = (_idxLED + 1) % 3; // Next LED => idxLED = 0,1,2,...
    count = _waitTime[_idxLED];
    //_secondCount = 0;
}
*/

int Traffic_Blink::getCount()
{
    return _secondCount + 1;
}

//----- class LED --------------------

// Hàm kiểm tra thời gian đã trôi qua - Non-Blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
    if (millis() - ulTimer < millisecond)
        return false;
    ulTimer = millis();
    return true;
}
// Định dạng chuỗi %s,%d,...
String StringFormat(const char *fmt, ...)
{
    va_list vaArgs;
    va_start(vaArgs, fmt);
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, vaArgs);
    const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
    va_end(vaArgsCopy);
    int iSize = iLen + 1;
    char *buff = (char *)malloc(iSize);
    vsnprintf(buff, iSize, fmt, vaArgs);
    va_end(vaArgs);
    String s = buff;
    free(buff);
    return String(s);
}