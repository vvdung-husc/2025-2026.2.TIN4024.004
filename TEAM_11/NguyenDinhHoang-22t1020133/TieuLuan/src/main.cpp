// --- THAY 3 DÒNG NÀY BẰNG MÃ TRÊN BLYNK CỦA BẠN ---
#define BLYNK_TEMPLATE_ID "TMPL6r03g4fU3"
#define BLYNK_TEMPLATE_NAME "BME280"
#define BLYNK_AUTH_TOKEN "Sfe6-iTQxcIoz_x4rGEjcLL5y8J4cUW3"

// In log ra Serial Monitor
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

// --- CẤU HÌNH OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- CẤU HÌNH DHT22 & BMP180 ---
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

// --- CẤU HÌNH WIFI WOKWI ---
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Timer của Blynk thay cho hàm delay()
BlynkTimer timer;

// Hàm đọc cảm biến và gửi dữ liệu
void sendSensor() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  float pres = bmp.readPressure() / 100.0;

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Lỗi: Không đọc được dữ liệu DHT22!");
    return;
  }

  // 1. Hiển thị OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(" TRAM THOI TIET BLYNK");
  display.println("---------------------");
  display.print("Nhiet do: "); display.print(temp); display.println(" C");
  display.print("Do am   : "); display.print(hum); display.println(" %");
  display.print("Ap suat : "); display.print(pres); display.println(" hPa");
  display.display();

  // 2. Gửi lên Blynk qua các chân ảo (Virtual Pins)
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, hum);
  Blynk.virtualWrite(V2, pres);
  
  Serial.println("Đã cập nhật dữ liệu lên Blynk!");
}

void setup() {
  Serial.begin(115200);

  // Khởi tạo các module
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Lỗi màn hình OLED"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);

  dht.begin();

  if (!bmp.begin()) {
    Serial.println("Lỗi BMP180!");
    while (1);
  }

  // Kết nối WiFi và hệ thống Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Cài đặt hẹn giờ: Gọi hàm sendSensor mỗi 2 giây (2000ms)
  timer.setInterval(2000L, sendSensor);
}

void loop() {
  // Chạy các tiến trình nền của Blynk và Timer
  Blynk.run();
  timer.run();
}