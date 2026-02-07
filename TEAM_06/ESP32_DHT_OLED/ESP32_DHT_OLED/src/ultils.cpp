#include "ultils.h"

// --- Helper Functions ---
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
    if (millis() - ulTimer < millisecond) return false;
    ulTimer = millis();
    return true;
}

// ================= MyLED Implementation =================
MyLED::MyLED() { _pin = -1; _ledState = LOW; _timer = 0; }

void MyLED::setup(int pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void MyLED::blink(uint32_t interval) {
    if (IsReady(_timer, interval)) {
        _ledState = !_ledState;
        digitalWrite(_pin, _ledState);
    }
}

void MyLED::off() { _ledState = LOW; digitalWrite(_pin, LOW); }
void MyLED::on() { _ledState = HIGH; digitalWrite(_pin, HIGH); }

// ================= MyDHT Implementation =================
MyDHT::MyDHT() { _dht = nullptr; _temp = 0.0; _hum = 0.0; _timer = 0; }

void MyDHT::setup(int pin, int type) {
    _pin = pin;
    _type = type;
    _dht = new DHT(_pin, _type);
    _dht->begin();
}

void MyDHT::run() {
    if (!IsReady(_timer, 2000)) return; // Đọc mỗi 2 giây
    float t = _dht->readTemperature();
    float h = _dht->readHumidity();
    if (!isnan(t)) _temp = t;
    if (!isnan(h)) _hum = h;
}

float MyDHT::getTemp() { return _temp; }
float MyDHT::getHum() { return _hum; }

// ================= MyOLED Implementation =================
MyOLED::MyOLED() { _display = nullptr; }

void MyOLED::setup(int sda, int scl, uint8_t address) {
    _address = address;
    Wire.begin(sda, scl);
    _display = new Adafruit_SSD1306(128, 64, &Wire, -1);
    
    if(!_display->begin(SSD1306_SWITCHCAPVCC, _address)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    _display->clearDisplay();
    _display->setTextColor(SSD1306_WHITE);
    _display->display();
}

void MyOLED::displayInfo(float temp, float hum, String statusMsg) {
    _display->clearDisplay();
    
    // --- DÒNG 1: Tiêu đề + Trạng thái (Giống ảnh) ---
    _display->setTextSize(1);
    _display->setCursor(0, 0);
    _display->print("Temperature: "); 
    _display->print(statusMsg);

    // --- DÒNG 2: Giá trị Nhiệt độ (Font TO) ---
    _display->setTextSize(2);
    _display->setCursor(0, 12); // Dịch xuống một chút
    _display->print(temp, 2);   // Lấy 2 số thập phân
    _display->setTextSize(1);   // Chỉnh nhỏ lại để viết ký hiệu độ
    _display->cp437(true);      // Bật mã ký tự mở rộng
    _display->write(167);       // Ký tự độ (°)
    _display->setTextSize(2);   // Trả lại font to
    _display->print("C");

    // --- DÒNG 3: Tiêu đề Độ ẩm ---
    _display->setTextSize(1);
    _display->setCursor(0, 35); // Vị trí dòng 3
    _display->print("Humidity:");

    // --- DÒNG 4: Giá trị Độ ẩm (Font TO) ---
    _display->setTextSize(2);
    _display->setCursor(0, 47); // Vị trí dòng 4
    _display->print(hum, 2);
    _display->print(" %");

    _display->display();
}

// ================= SystemController Implementation =================
SystemController::SystemController() { _timer = 0; }

void SystemController::run(MyLED &ledGreen, MyLED &ledYellow, MyLED &ledRed, MyDHT &dht, MyOLED &oled) {
    // 1. Cập nhật cảm biến
    dht.run();
    float t = dht.getTemp();
    float h = dht.getHum();

    // 2. Logic xác định trạng thái
    String statusMsg = "";
    int mode = 0; // 0: Green, 1: Yellow, 2: Red

    if (t < 13.0) {
        statusMsg = "TOO COLD"; mode = 0;
    } else if (t < 20.0) {
        statusMsg = "COLD"; mode = 0;
    } else if (t < 25.0) {
        statusMsg = "COOL"; mode = 1;
    } else if (t < 30.0) {
        statusMsg = "WARM"; mode = 1;
    } else if (t <= 35.0) {
        statusMsg = "HOT"; mode = 2;
    } else {
        statusMsg = "TOO HOT"; mode = 2;
    }

    // 3. Điều khiển LED
    switch (mode) {
        case 0: // Green
            ledGreen.blink(500); ledYellow.off(); ledRed.off(); break;
        case 1: // Yellow
            ledYellow.blink(500); ledGreen.off(); ledRed.off(); break;
        case 2: // Red
            ledRed.blink(200); ledGreen.off(); ledYellow.off(); break;
    }

    // 4. Cập nhật màn hình (mỗi 1s)
    if (IsReady(_timer, 1000)) {
        oled.displayInfo(t, h, statusMsg);
    }
}