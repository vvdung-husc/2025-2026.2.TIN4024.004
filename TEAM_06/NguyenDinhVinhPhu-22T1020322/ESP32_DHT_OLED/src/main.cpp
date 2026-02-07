#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- CẤU HÌNH CHÂN (Dựa trên diagram.json của Ngô Thời Quang) ---
#define PIN_DHT     16  // Dây dht1:SDA nối vào esp:16
#define DHT_TYPE    DHT22

#define PIN_GREEN   15  // led3 (Limegreen) nối vào esp:15
#define PIN_YELLOW  2   // led2 (Yellow) nối vào esp:2
#define PIN_RED     4   // led1 (Red) nối vào esp:4

// --- CẤU HÌNH CHÂN OLED (QUAN TRỌNG: Custom I2C) ---
#define PIN_OLED_SDA 13 // oled1:SDA nối vào esp:13
#define PIN_OLED_SCL 12 // oled1:SCL nối vào esp:12

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- KHỞI TẠO DHT ---
DHT dht(PIN_DHT, DHT_TYPE);

// --- BIẾN TOÀN CỤC ---
float t = 0; // Nhiệt độ
float h = 0; // Độ ẩm
unsigned long lastReadTime = 0; // Timer đọc cảm biến
unsigned long lastBlinkTime = 0; // Timer nháy đèn
bool ledState = false; // Trạng thái tắt/bật của đèn

void setup() {
  Serial.begin(115200);
  
  // 1. Setup Đèn
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_RED, OUTPUT);

  // 2. Setup DHT
  dht.begin();

  // 3. Setup OLED (LƯU Ý: Phải set chân I2C trước khi begin OLED)
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL); // Kích hoạt I2C trên chân 13 và 12

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED khởi động thất bại!"));
    for(;;);
  }
  
  // Màn hình chào mừng
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println("WEATHER STATION");
  display.setCursor(30, 40);
  display.println("Loading...");
  display.display();
  delay(1000); 
}

// Hàm lấy thông báo trạng thái dựa vào nhiệt độ
String getStatusMessage(float temp) {
    if (temp < 13) return "TOO COLD";
    if (temp < 20) return "COLD";
    if (temp < 25) return "COOL";
    if (temp < 30) return "WARM";
    if (temp <= 35) return "HOT";
    return "TOO HOT";
}

// Hàm xử lý nháy đèn (Logic cũ)
void blinkLedByTemp(float temp) {
  if (isnan(temp)) {
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_YELLOW, LOW);
    digitalWrite(PIN_RED, LOW);
    return;
  }

  // Timer: 500ms đảo trạng thái
  if (millis() - lastBlinkTime > 500) {
    lastBlinkTime = millis();
    ledState = !ledState;
    
    // Reset tắt hết
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_YELLOW, LOW);
    digitalWrite(PIN_RED, LOW);

    if (ledState) {
        if (temp < 20) {
            digitalWrite(PIN_GREEN, HIGH);
        } 
        else if (temp >= 20 && temp < 30) {
            digitalWrite(PIN_YELLOW, HIGH);
        } 
        else { // >= 30
            digitalWrite(PIN_RED, HIGH);
        }
    }
  }
}

void loop() {
  // 1. Đọc cảm biến mỗi 2 giây
  if (millis() - lastReadTime > 2000) {
    lastReadTime = millis();
    
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();

    if (isnan(newT) || isnan(newH)) {
      Serial.println("Lỗi đọc DHT22!");
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Sensor Error!");
      display.display();
    } else {
      t = newT;
      h = newH;
      
      // Hiển thị OLED
      display.clearDisplay();
      
      // Vẽ khung viền cho đẹp
      display.drawRect(0, 0, 128, 64, WHITE);
      
      // Dòng 1: Status
      display.setTextSize(1);
      display.setCursor(5, 5);
      display.print("STT: ");
      display.println(getStatusMessage(t));
      
      // Đường kẻ ngang
      display.drawLine(0, 18, 128, 18, WHITE);

      // Dòng 2: Temp
      display.setTextSize(2); // Chữ to
      display.setCursor(5, 25);
      display.print(t, 1);
      display.setTextSize(1); // Ký hiệu độ C nhỏ
      display.print("o");
      display.setTextSize(2);
      display.print("C");

      // Dòng 3: Humid
      display.setTextSize(1);
      display.setCursor(80, 25); // Đẩy sang phải
      display.print("Humid");
      display.setCursor(80, 40);
      display.print(h, 0);
      display.print("%");

      display.display();
    }
  }

  // 2. Nháy đèn (Chạy liên tục)
  blinkLedByTemp(t);
}