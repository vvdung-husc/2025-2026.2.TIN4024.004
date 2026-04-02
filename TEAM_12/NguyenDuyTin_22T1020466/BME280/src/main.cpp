#define BLYNK_TEMPLATE_ID "TMPL6nbN7KgOW"
#define BLYNK_TEMPLATE_NAME "BME280"
#define BLYNK_AUTH_TOKEN "5hXUF9fQfJUWPFkBQapuOOhABrnBA8ob"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"

// Cấu hình CHUẨN theo diagram.json của bạn
#define BUTTON_PIN 13  // Dây màu orange
#define LED_PIN    2   // Dây màu magenta
#define DHTPIN     15  // Dây màu green
#define DHTTYPE    DHT22

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool oledState = true;

// Hàm ngắt nút bấm (GPIO 13)
void IRAM_ATTR toggleOLED() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 250) { // Chống rung phím
    oledState = !oledState;
  }
  lastTime = millis();
}

void updateSystem() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float p = bmp.readPressure() / 100.0F;

  if (isnan(h) || isnan(t)) return;

  // Đẩy dữ liệu lên Blynk
  Blynk.virtualWrite(V1, t); // Datastream V1: Temp
  Blynk.virtualWrite(V2, h); // Datastream V2: Humi
  Blynk.virtualWrite(V3, p); // Datastream V3: Pres

  if (oledState) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("TIN'S STATION");
    display.printf("Nhiet: %.1f C\n", t);
    display.printf("Do am: %.1f %%\n", h);
    display.printf("Ap suat: %.1f hPa", p);
    display.display();
    digitalWrite(LED_PIN, HIGH); // Sáng LED khi màn hình bật
  } else {
    display.clearDisplay();
    display.display();
    digitalWrite(LED_PIN, LOW); // Tắt LED khi màn hình tắt
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(BUTTON_PIN, toggleOLED, FALLING);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  dht.begin();
  bmp.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, "Wokwi-GUEST", "");
  timer.setInterval(2000L, updateSystem);
}

void loop() {
  Blynk.run();
  timer.run();
}