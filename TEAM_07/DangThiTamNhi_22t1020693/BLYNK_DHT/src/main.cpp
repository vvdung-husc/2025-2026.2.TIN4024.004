#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Thông tin WiFi và Blynk
char auth[] = "YOUR_BLYNK_TOKEN";
char ssid[] = "Duc Thinh";
char pass[] = "0971527584";

// Cảm biến DHT
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long previousMillis = 0;
const long interval = 2000; // 2 giây

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Khởi động OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Khoi dong...");
  display.display();

  // Kết nối WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Kết nối Blynk
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Đọc cảm biến DHT
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Hiển thị lên OLED
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);
    display.print("Nhiet: ");
    display.print(temperature);
    display.println(" C");
    display.setCursor(0,30);
    display.print("Am: ");
    display.print(humidity);
    display.println(" %");
    display.display();

    // Gửi dữ liệu về Blynk
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);
  }
}