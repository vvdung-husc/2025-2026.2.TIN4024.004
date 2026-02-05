#include "utils.h"

LED::LED() {
    _pin = -1;
    _state = false;
    _timer = 0;
}

void LED::setup(int pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
}

bool LED::blink(unsigned long interval) {
    if (!IsReady(_timer, interval)) return false;
    _state = !_state;
    digitalWrite(_pin, _state ? HIGH : LOW);
    return _state;
}

void LED::off() {
    digitalWrite(_pin, LOW);
    _state = false;
}

DHT_OLED::DHT_OLED() : _dht(0, DHT22) {
    _temperature = 0;
    _humidity = 0;
    _status = "";
    _blinkLED = -1;
}

void DHT_OLED::setup(int pinDHT, int ledGreen, int ledYellow, int ledRed) {
    _dht = DHT(pinDHT, DHT22);
    _dht.begin();

    _ledGreen.setup(ledGreen);
    _ledYellow.setup(ledYellow);
    _ledRed.setup(ledRed);
}

void DHT_OLED::run(Adafruit_SSD1306& display) {
    static unsigned long timerRead = 0;

    blinkLED();

    if (!IsReady(timerRead, 2000)) return;

    _temperature = _dht.readTemperature();
    _humidity = _dht.readHumidity();

    updateLED();
    showOLED(display);
}

void DHT_OLED::updateLED() {
    _ledGreen.off();
    _ledYellow.off();
    _ledRed.off();

    if (_temperature < 20) {
        _status = "COLD";
        _blinkLED = 0;
    }
    else if (_temperature < 30) {
        _status = "WARM";
        _blinkLED = 1;
    }
    else {
        _status = "HOT";
        _blinkLED = 2;
    }
}

void DHT_OLED::blinkLED() {
    bool ledState = false;

    switch (_blinkLED) {
        case 0: ledState = _ledGreen.blink(500); break;
        case 1: ledState = _ledYellow.blink(500); break;
        case 2: ledState = _ledRed.blink(500); break;
        default: return;
    }

    if (ledState) {
        printf("Temp: %.1f C | Humi: %.1f %% | %s\n",
               _temperature, _humidity, _status);
    }
}

void DHT_OLED::showOLED(Adafruit_SSD1306& display) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.printf("Temp: %.1f C\n", _temperature);

    display.setCursor(0, 12);
    display.printf("Humi: %.1f %%\n", _humidity);

    display.setCursor(0, 30);
    display.setTextSize(2);
    display.print(_status);

    display.display();
}

bool IsReady(unsigned long& timer, uint32_t ms) {
    if (millis() - timer < ms) return false;
    timer = millis();
    return true;
}
