#include "utils.h"

// ===== LED =====
LED::LED() {
  _pin = -1;
  _state = 0;
  _timer = 0;
}

void LED::setup(int pin) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  _state = 0;
  _timer = millis();
}

void LED::off() {
  digitalWrite(_pin, LOW);
  _state = 0;
  _timer = millis();
}

// SÁNG → GIỮ → TẮT → LẶP
void LED::blink(uint32_t onTime, uint32_t holdTime, uint32_t offTime) {
  unsigned long now = millis();

  switch (_state) {
    case 0: // BẬT
      digitalWrite(_pin, HIGH);
      _timer = now;
      _state = 1;
      break;

    case 1: // GIỮ SÁNG
      if (now - _timer >= onTime + holdTime) {
        digitalWrite(_pin, LOW);
        _timer = now;
        _state = 2;
      }
      break;

    case 2: // TẮT
      if (now - _timer >= offTime) {
        _state = 0;
      }
      break;
  }
}

// ===== SYSTEM =====
DHT_OLED::DHT_OLED() : _dht(0, DHT22) {
  _temp = 0;
  _humi = 0;
  _status = "";
  _activeLED = -1;
  _sensorTimer = 0;
}

void DHT_OLED::setup(int pinDHT, int ledGreen, int ledYellow, int ledRed) {
  _dht = DHT(pinDHT, DHT22);
  _dht.begin();

  _green.setup(ledGreen);
  _yellow.setup(ledYellow);
  _red.setup(ledRed);
}

void DHT_OLED::run(Adafruit_SSD1306& display) {
  unsigned long now = millis();

  // đọc cảm biến mỗi 2 giây
  if (now - _sensorTimer >= 2000) {
    _sensorTimer = now;
    readSensor();
    updateStatus();
    showOLED(display);
  }

  if (_activeLED != -1) {
    blinkLED();
  }
}

void DHT_OLED::readSensor() {
  _temp = _dht.readTemperature();
  _humi = _dht.readHumidity();
}

// ===== NGƯỠNG =====
void DHT_OLED::updateStatus() {
  if (_temp < 13) {
    _status = "TOO COLD";
    _activeLED = 0;
  }
  else if (_temp < 20) {
    _status = "COLD";
    _activeLED = 0;
  }
  else if (_temp < 25) {
    _status = "COOL";
    _activeLED = 1;
  }
  else if (_temp < 30) {
    _status = "WARM";
    _activeLED = 1;
  }
  else if (_temp < 35) {
    _status = "HOT";
    _activeLED = 2;
  }
  else {
    _status = "TOO HOT";
    _activeLED = 2;
  }
}

// ===== CHỈ TẮT LED KHÔNG ACTIVE =====
void DHT_OLED::blinkLED() {
  switch (_activeLED) {
    case 0:
      _green.blink(100, 800, 600);
      _yellow.off();
      _red.off();
      break;

    case 1:
      _yellow.blink(100, 800, 600);
      _green.off();
      _red.off();
      break;

    case 2:
      _red.blink(100, 800, 600);
      _green.off();
      _yellow.off();
      break;
  }
}

void DHT_OLED::showOLED(Adafruit_SSD1306& display) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("Temp: %.1f C\n", _temp);

  display.setCursor(0, 12);
  display.printf("Humi: %.1f %%\n", _humi);

  display.setTextSize(2);
  display.setCursor(0, 32);
  display.print(_status);

  display.display();
}