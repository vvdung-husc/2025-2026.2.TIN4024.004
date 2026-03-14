#include <Arduino.h>
#include <U8g2lib.h>
#include <DHT.h>

// 1. Khai báo chân Pin cho ESP8266 V3
#define DHTPIN 00      // Chân D3 (GPIO0)
#define DHTTYPE DHT22 // Đổi thành DHT22 nếu bạn dùng màu trắng
#define LED_PIN D6    // Chân D6 (GPIO12)

// 2. Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  u8g2.begin();
  Serial.println("He thong ESP8266: San sang!");
}

void loop() {
  // Đọc dữ liệu từ DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Kiểm tra nếu cảm biến bị lỗi đọc
  if (isnan(h) || isnan(t)) {
    Serial.println("Loi doc cam bien DHT!");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, 30);
    u8g2.print("Sensor Error!");
    u8g2.sendBuffer();
  } else {
    // Hiển thị Serial
    Serial.printf("Nhiet do: %.1fC | Do am: %.1f%%\n", t, h);

    // Hiển thị OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr); // Font chữ to hơn
    
    u8g2.setCursor(0, 25);
    u8g2.print("Nhiệt Độ: "); u8g2.print(t); u8g2.print(" C");
    
    u8g2.setCursor(0, 55);
    u8g2.print("Độ Ẩm: "); u8g2.print(h); u8g2.print(" %");
    
    u8g2.sendBuffer();
  }

  // Điều khiển LED nhấp nháy mạnh hơn
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}