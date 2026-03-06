// 1. BẮT BUỘC KHAI BÁO BLYNK TRÊN CÙNG
#define BLYNK_TEMPLATE_ID "TMPL6MmOgzaXh"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "kDc6eUsp8Gz4F_BA5JCIo7TPo_ZZOioM"
#define BLYNK_PRINT Serial

// 2. INCLUDE CÁC THƯ VIỆN CẦN THIẾT
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// --- Cấu hình WiFi ---
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// --- Định nghĩa chân kết nối ---
#define PIN_LED      21
#define PIN_BTN      23
#define PIN_TM_CLK   18
#define PIN_TM_DIO   19
#define PIN_DHT      16
#define DHT_TYPE     DHT22

// =========================================================
// CÁC HÀM VÀ CLASS TIỆN ÍCH
// =========================================================

// Hàm xử lý thời gian không dùng delay
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
    if (millis() - ulTimer < millisecond) return false;
    ulTimer = millis();
    return true;
}

class MyLED {
public:
    MyLED() { _pin = -1; _state = false; }
    void setup(int pin) {
        _pin = pin;
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
    }
    void set(bool state) {
        _state = state;
        if (_state) digitalWrite(_pin, HIGH);
        else digitalWrite(_pin, LOW);
    }
    bool getState() { return _state; }
private:
    int _pin;
    bool _state;
};

class MyButton {
public:
    MyButton() { _pin = -1; _lastState = HIGH; _debounceTimer = 0; }
    void setup(int pin) {
        _pin = pin;
        pinMode(_pin, INPUT_PULLUP);
    }
    bool isPressed() {
        bool currentState = digitalRead(_pin);
        bool pressed = false;
        if (currentState != _lastState) {
            if (IsReady(_debounceTimer, 50)) {
                if (currentState == LOW) pressed = true;
                _lastState = currentState;
            }
        }
        return pressed;
    }
private:
    int _pin;
    bool _lastState;
    unsigned long _debounceTimer;
};

class MyDHT {
public:
    MyDHT() { _dht = NULL; _temp = 0.0; _hum = 0.0; _timer = 0; }
    void setup(int pin, int type) {
        _pin = pin;
        _type = type;
        _dht = new DHT(_pin, _type);
        _dht->begin();
    }
    void run() {
        if (!IsReady(_timer, 2000)) return;
        float t = _dht->readTemperature();
        float h = _dht->readHumidity();
        if (!isnan(t)) _temp = t;
        if (!isnan(h)) _hum = h;
    }
    float getTemp() { return _temp; }
    float getHum() { return _hum; }
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
    MyDisplay() { _tm = NULL; }
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

    unsigned long getUptime() { return _uptime; }

    bool getBtnTriggered() {
        if (_btnTriggered) {
            _btnTriggered = false;
            return true;
        }
        return false;
    }

    void run(MyLED &led, MyButton &btn, MyDHT &dht, MyDisplay &display) {
        // Nút nhấn cứng
        if (btn.isPressed()) {
            _isLedOn = !_isLedOn;
            led.set(_isLedOn);
            _btnTriggered = true;
        }

        // Đọc DHT
        dht.run();

        // Cập nhật Uptime mỗi giây
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

// =========================================================
// KHỞI TẠO ĐỐI TƯỢNG VÀ CHƯƠNG TRÌNH CHÍNH
// =========================================================

MyLED ledBlue;
MyButton btnToggle;
MyDHT dhtSensor;
MyDisplay tmDisplay;
SystemController mySystem;

// Xử lý khi nhấn nút trên App Blynk (chân V1)
BLYNK_WRITE(V1) {
    int pinValue = param.asInt(); 
    if (pinValue == 1) {
        mySystem.setLedState(true, ledBlue);
    } else {
        mySystem.setLedState(false, ledBlue);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Bat dau khoi dong...");

    // 1. Cài đặt phần cứng
    ledBlue.setup(PIN_LED);
    btnToggle.setup(PIN_BTN);
    dhtSensor.setup(PIN_DHT, DHT_TYPE);
    tmDisplay.setup(PIN_TM_CLK, PIN_TM_DIO);

    // 2. Kết nối WiFi thủ công để fix lỗi DNS trên Wokwi
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Đợi 2 giây cho môi trường mạng ảo thật sự ổn định DNS
    delay(2000); 
    Serial.println("Connecting to Blynk...");

    // 3. Khởi tạo Blynk sau khi đã chắc chắn có mạng
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
}

void loop() {
    Blynk.run();
    
    // Chạy logic của hệ thống
    mySystem.run(ledBlue, btnToggle, dhtSensor, tmDisplay);

    // Đồng bộ nút cứng lên Blynk
    if (mySystem.getBtnTriggered()) {
        if (ledBlue.getState()) {
            Blynk.virtualWrite(V1, 1);
        } else {
            Blynk.virtualWrite(V1, 0);
        }
    }

    // Gửi data định kỳ lên Blynk
    if (mySystem.hasDataToUpdate()) {
        Blynk.virtualWrite(V0, mySystem.getUptime());
        Blynk.virtualWrite(V2, dhtSensor.getTemp());
        Blynk.virtualWrite(V3, dhtSensor.getHum());
    }
}