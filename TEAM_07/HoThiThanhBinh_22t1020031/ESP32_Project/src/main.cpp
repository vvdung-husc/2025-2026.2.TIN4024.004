#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// --- CẤU HÌNH CHÂN (PINS) ---
#define DHTPIN 0
#define DHTTYPE DHT22  // Đổi thành DHT22 nếu cảm biến thực tế của bạn là loại màu trắng
#define MQ2PIN A0      // Chân A0 (Đọc tín hiệu Analog từ MQ2)
#define LED_PIN 2      // Chân D4 (GPIO2 - LED Builtin trên NodeMCU)

// Khởi tạo đối tượng cảm biến DHT
DHT dht(DHTPIN, DHTTYPE);

// Khởi tạo màn hình OLED SH1106 (Sử dụng Hardware I2C: D1=SCL, D2=SDA)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// --- CÁC BIẾN THỜI GIAN VÀ TRẠNG THÁI ---
unsigned long lastSensorRead = 0;
unsigned long lastBlink = 0;
bool ledState = false;

float temp = 0.0;
float hum = 0.0;
int gasValue = 0;

void setup() {
  Serial.begin(115200);
  
  // Cấu hình LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Tắt LED (LED Builtin của ESP8266 thường sáng ở mức LOW)
  
  // Khởi động cảm biến
  dht.begin();
  
  // Khởi động OLED
  u8g2.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. NHẤP NHÁY LED LIÊN TỤC MỖI 500ms
  if (currentMillis - lastBlink >= 500) {
    lastBlink = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? LOW : HIGH); 
  }

  // 2. ĐỌC CẢM BIẾN VÀ CẬP NHẬT OLED MỖI 2 GIÂY
  if (currentMillis - lastSensorRead >= 2000) {
    lastSensorRead = currentMillis;

    // Đọc dữ liệu
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    gasValue = analogRead(MQ2PIN); // Trả về giá trị từ 0 - 1023

    // Kiểm tra lỗi DHT
    if (isnan(hum) || isnan(temp)) {
      Serial.println("Lỗi: Không đọc được dữ liệu từ DHT!");
    } else {
      Serial.printf("Nhiệt độ: %.1f C | Độ ẩm: %.1f %% | Gas: %d\n", temp, hum, gasValue);
    }

    // 3. HIỂN THỊ LÊN MÀN HÌNH OLED
    u8g2.clearBuffer();          
    u8g2.setFont(u8g2_font_ncenB08_tr); // Font chữ mặc định dễ nhìn
    
    // In tiêu đề (CĂN GIỮA)
    const char* title = "Truong DHKH - IOT";
    int titleX = (128 - u8g2.getStrWidth(title)) / 2;
    u8g2.setCursor(titleX, 12); 
    u8g2.print(title);
    
    // In tên nhóm (CĂN GIỮA)
    const char* group = "- Nhom 7 -";
    int groupX = (128 - u8g2.getStrWidth(group)) / 2;
    u8g2.setCursor(groupX, 26); 
    u8g2.print(group);
    
    // In thông số
    u8g2.setCursor(0, 42); 
    u8g2.print("Nhiet do: "); u8g2.print(temp, 1); u8g2.print(" C");
    
    u8g2.setCursor(0, 54); 
    u8g2.print("Do am: "); u8g2.print(hum, 1); u8g2.print(" %");
    
    u8g2.setCursor(0, 64); 
    u8g2.print("Khi GAS: "); u8g2.print(gasValue);
    
    u8g2.sendBuffer(); // Bắn dữ liệu lên màn hình
  }
}