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

  // --- HIỆU ỨNG CHÀO MỪNG ---
  String welcomeMsg = "TEAM 04.004 Sensor System";
  u8g2.setFont(u8g2_font_ncenB14_tr); 
  int msgWidth = u8g2.getUTF8Width(welcomeMsg.c_str()); 
  
  for (int x = 128; x > -msgWidth; x -= 3) { 
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.setCursor(x, 42); 
    u8g2.print(welcomeMsg);
    u8g2.sendBuffer();
    delay(10); 
  }
}

void loop() {
  // Tính toán UpTime
  unsigned long totalSeconds = millis() / 1000;
  int hours = totalSeconds / 3600;
  int minutes = (totalSeconds % 3600) / 60;
  int seconds = totalSeconds % 60;

  // Đọc cảm biến
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);

  // ĐỒNG NHẤT FONT CHỮ CHO TOÀN BỘ MÀN HÌNH CHÍNH
  u8g2.setFont(u8g2_font_6x12_tr); 

  // 1. Phần Top (Tiêu đề)
  u8g2.drawStr(10, 14, "LeeJ Sensor System"); 
  u8g2.drawHLine(0, 18, 128);

  // 2. Phần Middle (Thông số)
  if (isnan(h) || isnan(t)) {
    u8g2.drawStr(25, 38, "Sensor Error!");
  } else {
    u8g2.setCursor(10, 33);
    u8g2.print("Nhiet do: "); u8g2.print(t, 1); u8g2.print(" C");
    
    u8g2.setCursor(10, 46);
    u8g2.print("Do am   : "); u8g2.print(h, 1); u8g2.print(" %");
  }

  // 3. Phần Bottom (UpTime)
  u8g2.drawHLine(0, 51, 128); 
  u8g2.setCursor(10, 61);
  u8g2.print("UpTime  : ");
  if(hours < 10) u8g2.print("0"); u8g2.print(hours); u8g2.print(":");
  if(minutes < 10) u8g2.print("0"); u8g2.print(minutes); u8g2.print(":");
  if(seconds < 10) u8g2.print("0"); u8g2.print(seconds);

  u8g2.sendBuffer();

  // LED nháy
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(900); 
}