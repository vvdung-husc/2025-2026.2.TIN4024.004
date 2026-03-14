#include <Arduino.h>
#include <U8g2lib.h>
#include <DHT.h>

// Cấu hình chân Pin cho ESP8266 V3
#define DHTPIN 0       // Chân D3 (GPIO0)
#define DHTTYPE DHT22 
#define LED_PIN 12     // Chân D6 (GPIO12)

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  u8g2.begin();

  // --- HIỆU ỨNG CHỮ CHẠY CỠ LỚN (RÚT GỌN) ---
  // Nội dung ngắn gọn, chuyên nghiệp hơn
  String welcomeMsg = "LeeJ Sensor System - Powered By Thanh Doanh";
  
  // Font ncenB14 chiếm khoảng 1/2 chiều cao màn hình
  u8g2.setFont(u8g2_font_ncenB14_tr); 
  int msgWidth = u8g2.getUTF8Width(welcomeMsg.c_str()); 
  
  // Hiệu ứng chạy chữ
  for (int x = 128; x > -msgWidth; x -= 3) { 
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64); // Vẽ khung bao quanh
    
    // Y=42 để chữ nằm ở trung tâm khung dọc
    u8g2.setCursor(x, 42); 
    u8g2.print(welcomeMsg);
    
    u8g2.sendBuffer();
    delay(10); 
  }
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64); // Vẽ khung chính

  if (isnan(h) || isnan(t)) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(20, 35, "Sensor Error!");
  } else {
    // Tiêu đề Header
    u8g2.setFont(u8g2_font_6x12_t_symbols);
    u8g2.drawStr(12, 15, "LeeJ Sensor System"); 
    u8g2.drawHLine(0, 20, 128); // Kẻ ngang phân cách

    // Hiển thị số liệu
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(15, 40);
    u8g2.print("Nhiet do: "); u8g2.print(t, 1); u8g2.print(" C");
    
    u8g2.setCursor(15, 58);
    u8g2.print("Do am   : "); u8g2.print(h, 1); u8g2.print(" %");
  }

  u8g2.sendBuffer();

  // LED nháy báo hiệu hoạt động
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
  delay(1700);
}