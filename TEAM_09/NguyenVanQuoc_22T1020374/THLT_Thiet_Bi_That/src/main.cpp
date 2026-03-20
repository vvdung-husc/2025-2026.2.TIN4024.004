#include <Arduino.h>
#include <U8g2lib.h>
#include <DHT.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// 1. Cấu hình OLED SH1106 (I2C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// 2. Cấu hình DHT
#define DHTPIN 0      // Chân D3 trên board tương ứng GPIO 0
#define DHTTYPE DHT11 // Đổi thành DHT22 nếu bạn dùng loại trắng
DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN 12 // D6 tương ứng với GPIO 12

// 3. Cấu hình MQ2
#define MQ2PIN A0

void setup() {
  // Cấu hình chân D6 là đầu ra (OUTPUT)
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  dht.begin();
  u8g2.begin();
  
  pinMode(LED_BUILTIN, OUTPUT); // Đèn led trên board (D4)
}

void loop() {
  digitalWrite(LED_PIN, HIGH); // Bật đèn (Cấp nguồn 3.3V)
  delay(500);                  // Đợi 0.5 giây
  digitalWrite(LED_PIN, LOW);  // Tắt đèn (Về 0V)
  delay(500);                  // Đợi 0.5 giây

  // Đọc dữ liệu
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Kiểm tra lỗi cảm biến
  if (isnan(h) || isnan(t)) {
    Serial.println("Lỗi đọc cảm biến DHT!");
    return;
  }

  // Hiển thị ra Serial Monitor để kiểm tra
  Serial.print("Nhiet do: "); Serial.print(t);
  Serial.print("C - Do am: "); Serial.print(h);

  // Hiển thị lên màn hình OLED
  u8g2.clearBuffer();					
  u8g2.setFont(u8g2_font_ncenB08_tr);	
  
  u8g2.setCursor(0, 30);
  u8g2.print("Temp: "); u8g2.print(t); u8g2.print(" C");
  
  u8g2.setCursor(0, 55);
  u8g2.print("Humi: "); u8g2.print(h); u8g2.print(" %");
  
  u8g2.sendBuffer();

  // Nháy LED trên board mỗi chu kỳ
  digitalWrite(LED_BUILTIN, LOW); 
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1500); 
}