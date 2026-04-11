#define BLYNK_TEMPLATE_ID "TMPL6nbN7KgOW"
#define BLYNK_TEMPLATE_NAME "BME280"
#define BLYNK_AUTH_TOKEN "5hXUF9fQfJUWPFkBQapuOOhABrnBA8ob"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// --- HIỆU CHỈNH CHÂN CẮM KHỚP VỚI JSON ---
#define DHTPIN 16     // Khớp với [ "dht1:SDA", "esp:16" ] trong JSON
#define DHTTYPE DHT22
#define PRES_PIN 34   // Khớp với [ "pot1:SIG", "esp:34" ] trong JSON

// Chân I2C cho OLED (JSON đang dùng chân 12 và 13 thay vì chân mặc định 21, 22)
#define OLED_SDA 13   // Khớp với [ "oled1:SDA", "esp:13" ]
#define OLED_SCL 12   // Khớp với [ "oled1:SCL", "esp:12" ]

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
BlynkTimer timer;

void updateSystem() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int rawAnalog = analogRead(PRES_PIN);
  float p = map(rawAnalog, 0, 4095, 950, 1050);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 0);
  display.println("WEATHER STATION");
  display.drawLine(0, 12, 128, 12, SSD1306_WHITE);
  
  display.setCursor(0, 25);

  if (isnan(h) || isnan(t)) {
    Serial.println("-> Sensor Error: Check Wiring!");
    display.println("Temp:  Error");
    display.println("Humid: Error");
  } else {
    Serial.printf("T: %.1f | H: %.1f | P: %.0f\n", t, h, p);
    display.printf("Temp:  %.1f C\n", t);
    display.printf("Humid: %.1f %%\n", h);
    
    Blynk.virtualWrite(V1, t); 
    Blynk.virtualWrite(V2, h); 
  }
  
  display.printf("Press: %.0f hPa", p);
  Blynk.virtualWrite(V3, p); 
  display.display();
}

void setup() {
  Serial.begin(115200);

  // 1. Khởi tạo lại I2C với các chân tùy chỉnh trong JSON
  Wire.begin(OLED_SDA, OLED_SCL);

  // 2. Khởi tạo OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED failed"));
  } else {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    display.println("System Starting...");
    display.display();
  }
  
  dht.begin();
  delay(2000); 

  // 3. Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, "Wokwi-GUEST", "");
  
  timer.setInterval(2000L, updateSystem);
}

void loop() {
  Blynk.run();
  timer.run();
}