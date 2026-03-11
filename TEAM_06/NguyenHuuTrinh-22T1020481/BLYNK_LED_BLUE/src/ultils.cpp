#include "ultils.h"
#include <BlynkSimpleEsp32.h>

// --- Helper Functions ---
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
    if (millis() - ulTimer < millisecond) return false;
    ulTimer = millis();
    return true;
}

// ================= MyLED =================
MyLED::MyLED() {
    _pin = -1;
    _state = false;
}

void MyLED::setup(int pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void MyLED::set(bool state) {
    _state = state;
    if (_state == true) {
        digitalWrite(_pin, HIGH);
    } else {
        digitalWrite(_pin, LOW);
    }
}

bool MyLED::getState() {
    return _state;
}

// ================= MyButton =================
MyButton::MyButton() {
    _pin = -1;
    _lastState = HIGH;
    _debounceTimer = 0;
}

void MyButton::setup(int pin) {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
}

bool MyButton::isPressed() {
    bool currentState = digitalRead(_pin);
    bool pressed = false;

    // Chống nhiễu nút nhấn (Debounce cơ bản)
    if (currentState != _lastState) {
        if (IsReady(_debounceTimer, 50)) {
            if (currentState == LOW) {
                pressed = true;
            }
            _lastState = currentState;
        }
    }
    return pressed;
}

// ================= MyDHT =================
MyDHT::MyDHT() {
    _dht = nullptr;
    _temp = 0.0;
    _hum = 0.0;
    _timer = 0;
}

void MyDHT::setup(int pin, int type) {
    _pin = pin;
    _type = type;
    _dht = new DHT(_pin, _type);
    _dht->begin();
}

void MyDHT::run() {
    if (IsReady(_timer, 2000) == false) return;

    float t = _dht->readTemperature();
    float h = _dht->readHumidity();

    if (isnan(t) == false) {
        _temp = t;
    }
    if (isnan(h) == false) {
        _hum = h;
    }
}

float MyDHT::getTemp() { return _temp; }
float MyDHT::getHum() { return _hum; }

// ================= MyDisplay (TM1637) =================
MyDisplay::MyDisplay() {
    _tm = nullptr;
}

void MyDisplay::setup(int clk, int dio) {
    _tm = new TM1637Display(clk, dio);
    _tm->setBrightness(0x0f); // Sáng tối đa
    _tm->clear();
}

void MyDisplay::showNumber(int num) {
    _tm->showNumberDec(num);
}

void MyDisplay::clear() {
    _tm->clear();
}

// ================= SystemController =================
SystemController::SystemController() {
    _timer1s = 0;
    _uptime = 0;
    _isLedOn = false;
}

void SystemController::setLedState(bool state, MyLED &led) {
    _isLedOn = state;
    led.set(_isLedOn);
}

void SystemController::run(MyLED &led, MyButton &btn, MyDHT &dht, MyDisplay &display) {
    // 1. Kiểm tra nút nhấn cứng trên mạch
    if (btn.isPressed() == true) {
        _isLedOn = !_isLedOn; // Đảo trạng thái
        led.set(_isLedOn);
        
        // Cập nhật lên Blynk (chân V1)
        if (_isLedOn == true) {
            Blynk.virtualWrite(V1, 1);
        } else {
            Blynk.virtualWrite(V1, 0);
        }
    }

    // 2. Đọc cảm biến
    dht.run();

    // 3. Mỗi 1 giây: Cập nhật Uptime, TM1637 và gửi dữ liệu lên Blynk
    if (IsReady(_timer1s, 1000) == true) {
        _uptime = _uptime + 1;
        
        // Hiển thị thời gian lên LED 7 đoạn
        display.showNumber(_uptime);

        // Gửi dữ liệu lên Blynk
        Blynk.virtualWrite(V0, _uptime);
        Blynk.virtualWrite(V2, dht.getTemp());
        Blynk.virtualWrite(V3, dht.getHum());
        
        Serial.printf("Uptime: %lu | Temp: %.1f | Hum: %.1f\n", _uptime, dht.getTemp(), dht.getHum());
    }
}