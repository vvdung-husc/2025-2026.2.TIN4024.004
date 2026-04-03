#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// Định nghĩa chân cắm theo sơ đồ Board thực hành
#define LED_BOARD D4   // LED Build-in (Active Low trên NodeMCU)
#define LED_DIR   D6   
#define DHTPIN    D3
#define DHTTYPE   DHT22
#define MQ2       A0

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void displayData(float t, float h, int g) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf); // Font nhỏ gọn, dễ nhìn
  u8g2.setCursor(0, 15); u8g2.printf("Temp: %.1f C", t);
  u8g2.setCursor(0, 35); u8g2.printf("Hum:  %.1f %%", h);
  u8g2.setCursor(0, 55); u8g2.printf("Gas:  %d", g);
  u8g2.sendBuffer();
}

void setup() {
  pinMode(LED_BOARD, OUTPUT);
  pinMode(LED_DIR, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  u8g2.begin();
}

void loop() {
  // Điều khiển LED đối nghịch
  bool state = (millis() / 500) % 2; 
  digitalWrite(LED_DIR, state);
  digitalWrite(LED_BOARD, !state); // LED_BOARD thường ngược logic (LOW là sáng)

  // Đọc cảm biến
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int gas = analogRead(MQ2);

  // Gửi Serial để debug
  Serial.printf("T: %.1f | H: %.1f | G: %d\n", temp, hum, gas);

  // Hiển thị OLED
  displayData(temp, hum, gas);

  delay(1000); // Cập nhật mỗi giây
}