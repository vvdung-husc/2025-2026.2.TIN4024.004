#define BLYNK_TEMPLATE_ID "TMPL6MmOgzaXh"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "kDc6eUsp8Gz4F_BA5JCIo7TPo_ZZOioM"
#define BLYNK_PRINT Serial

#include "WiFi.h"
#include "WiFiClient.h"
#include "BlynkSimpleEsp32.h"
#include "DHT.h"
#include "TM1637Display.h"

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define PIN_LED      21
#define PIN_BTN      23
#define PIN_TM_CLK   18
#define PIN_TM_DIO   19
#define PIN_DHT      16
#define DHT_TYPE     DHT22

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
    if (millis() - ulTimer < millisecond) {
        return false;
    }
    ulTimer = millis();
    return true;
}

class MyLED {
public:
    MyLED() { 
        _pin = -1; 
        _state = false; 
    }
    void setup(int pin) {
        _pin = pin;
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
    }
    void set(bool state) {
        _state = state;
        if (_state) {
            digitalWrite(_pin, HIGH);
        } else {
            digitalWrite(_pin, LOW);
        }
    }
    bool getState() { 
        return _state; 
    }
private:
    int _pin;
    bool _state;
};

class MyButton {
public:
    MyButton() { 
        _pin = -1; 
        _lastState = HIGH; 
        _buttonState = HIGH; 
        _lastDebounceTime = 0; 
    }
    void setup(int pin) {
        _pin = pin;
        pinMode(_pin, INPUT_PULLUP);
    }
    bool isPressed() {
        bool reading = digitalRead(_pin);
        bool pressed = false;
        
        if (reading != _lastState) {
            _lastDebounceTime = millis();
        }
        
        if ((millis() - _lastDebounceTime) > 50) {
            if (reading != _buttonState) {
                _buttonState = reading;
                if (_buttonState == LOW) {
                    pressed = true;
                }
            }
        }
        _lastState = reading;
        return pressed;
    }
private:
    int _pin;
    bool _lastState;
    bool _buttonState;
    unsigned long _lastDebounceTime;
};
class MyDHT {
public:
    MyDHT() { 
        _dht = NULL; 
        _temp = 0.0; 
        _hum = 0.0; 
        _timer = 0; 
    }
    void setup(int pin, int type) {
        _pin = pin;
        _type = type;
        _dht = new DHT(_pin, _type);
        _dht->begin();
    }
    void run() {
        if (IsReady(_timer, 2000)) {
            float t = _dht->readTemperature();
            float h = _dht->readHumidity();
            if (!isnan(t)) {
                _temp = t;
            }
            if (!isnan(h)) {
                _hum = h;
            }
        }
    }
    float getTemp() { 
        return _temp; 
    }
    float getHum() { 
        return _hum; 
    }
private:
    DHT* _dht;
    int _pin;
    int _type;
    float _temp;
    float _hum;
    unsigned long _timer;
};

class MyDisplay {
public:
    MyDisplay() { 
        _tm = NULL; 
    }
    void setup(int clk, int dio) {
        _tm = new TM1637Display(clk, dio);
        _tm->setBrightness(0x0f);
        _tm->clear();
    }
    void showNumber(int num) {
        _tm->showNumberDec(num);
    }
private:
    TM1637Display* _tm;
};

class SystemController {
public:
    SystemController() {
        _timer1s = 0;
        _uptime = 0;
        _isLedOn = false;
        _dataReady = false;
        _btnTriggered = false;
    }

    void setLedState(bool state, MyLED &led) {
        _isLedOn = state;
        led.set(_isLedOn);
    }

    bool hasDataToUpdate() {
        if (_dataReady) {
            _dataReady = false;
            return true;
        }
        return false;
    }

    unsigned long getUptime() { 
        return _uptime; 
    }

    bool getBtnTriggered() {
        if (_btnTriggered) {
            _btnTriggered = false;
            return true;
        }
        return false;
    }

    void run(MyLED &led, MyButton &btn, MyDHT &dht, MyDisplay &display) {
        if (btn.isPressed()) {
            if (_isLedOn) {
                _isLedOn = false;
            } else {
                _isLedOn = true;
            }
            led.set(_isLedOn);
            _btnTriggered = true;
        }

        dht.run();

        if (IsReady(_timer1s, 1000)) {
            _uptime++;
            display.showNumber(_uptime);
            _dataReady = true;
        }
    }
private:
    unsigned long _timer1s;
    unsigned long _uptime;
    bool _isLedOn;
    bool _dataReady;
    bool _btnTriggered;
};

MyLED ledBlue;
MyButton btnToggle;
MyDHT dhtSensor;
MyDisplay tmDisplay;
SystemController mySystem;

// Xử lý V0 - Switch điều khiển đèn
BLYNK_WRITE(V0) {
    int pinValue = param.asInt(); 
    if (pinValue == 1) {
        mySystem.setLedState(true, ledBlue);
    } else {
        mySystem.setLedState(false, ledBlue);
    }
}

void setup() {
    Serial.begin(115200);

    ledBlue.setup(PIN_LED);
    btnToggle.setup(PIN_BTN);
    dhtSensor.setup(PIN_DHT, DHT_TYPE);
    tmDisplay.setup(PIN_TM_CLK, PIN_TM_DIO);

    WiFi.begin(ssid, pass);
    Blynk.config(BLYNK_AUTH_TOKEN);
}

void loop() {
    // Phần cứng luôn chạy mà không bị nghẽn mạng
    mySystem.run(ledBlue, btnToggle, dhtSensor, tmDisplay);

    if (WiFi.status() == WL_CONNECTED) {
        Blynk.run();

        // Đồng bộ trạng thái đèn lên App qua V0
        if (mySystem.getBtnTriggered()) {
            if (ledBlue.getState()) {
                Blynk.virtualWrite(V0, 1);
            } else {
                Blynk.virtualWrite(V0, 0);
            }
        }

        // Cập nhật V1(Nhiệt độ), V2(Độ ẩm), V3(Thời gian)
        if (mySystem.hasDataToUpdate()) {
            Blynk.virtualWrite(V3, mySystem.getUptime());
            Blynk.virtualWrite(V1, dhtSensor.getTemp());
            Blynk.virtualWrite(V2, dhtSensor.getHum());
        }
    }
}