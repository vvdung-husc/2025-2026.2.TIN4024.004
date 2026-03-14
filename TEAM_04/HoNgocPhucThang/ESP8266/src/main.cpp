#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "DHT.h"

#define DHTPIN 00         
#define DHTTYPE DHT22    
#define RELAY_PIN D6     

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastBlink = 0;
unsigned long lastSensor = 0;
bool ledState = false;
float h = 0, t = 0;
int gasValue = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  dht.begin();
  u8g2.begin();
}

void loop() {
  unsigned long now = millis();

  // 1. Nháy đèn tín hiệu D6
  if (now - lastBlink >= 500) {
    lastBlink = now;
    ledState = !ledState;
    digitalWrite(RELAY_PIN, ledState);
  }

  // 2. Đọc cảm biến mỗi 2 giây
  if (now - lastSensor >= 2000) {
    lastSensor = now;
    h = dht.readHumidity();
    t = dht.readTemperature();
    gasValue = analogRead(A0);
  }

  // 3. VẼ GIAO DIỆN DẠNG BẢNG
  u8g2.clearBuffer();
  
  // Vẽ khung viền ngoài cùng cho bảng
  u8g2.drawFrame(0, 0, 128, 64);
  
  // Vẽ các đường kẻ ngang chia hàng
  u8g2.drawHLine(0, 20, 128); // Đường kẻ sau tiêu đề
  u8g2.drawHLine(0, 42, 128); // Đường kẻ chia giữa các thông số
  
  // Vẽ đường kẻ dọc chia cột (nếu muốn chia đôi hàng)
  u8g2.drawVLine(64, 20, 22); // Chia đôi hàng nhiệt độ và độ ẩm

  // Nội dung Tiêu đề
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(12, 14, "HE THONG ESP8266");

  // Hàng 1: Nhiệt độ & Độ ẩm
  u8g2.setCursor(5, 35);
  u8g2.print("T:"); u8g2.print(t, 1); u8g2.print("C");
  
  u8g2.setCursor(69, 35);
  u8g2.print("H:"); u8g2.print(h, 1); u8g2.print("%");

  // Hàng 2: Khí Gas
  u8g2.setCursor(5, 57);
  u8g2.print("Khi Gas: "); u8g2.print(gasValue);
  
  // Trạng thái hệ thống (Nhấp nháy trong bảng)
  if(ledState) {
    u8g2.drawStr(85, 57, "[LED]");
  }

  u8g2.sendBuffer();
}