#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- Cấu hình chân ---
#define PIN_DHT     16
#define DHT_TYPE    DHT22
#define PIN_RELAY   2   // Chân điều khiển Relay quạt

#define TEMP_THRESHOLD 30.0 // Ngưỡng bật quạt (30 độ C)
#define HYSTERESIS     1.5  // Độ trễ để quạt chạy ổn định

// --- Khởi tạo ---
DHT dht(PIN_DHT, DHT_TYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
    Serial.begin(115200);
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW); // Mặc định quạt tắt

    dht.begin();
    Wire.begin(13, 12); // Chân SDA=13, SCL=12 theo diagram cũ của mày
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        for(;;);
    }
}

void loop() {
    static unsigned long lastRead = 0;
    if (millis() - lastRead > 2000) {
        lastRead = millis();
        
        float t = dht.readTemperature();
        if (!isnan(t)) {
            // Logic điều khiển quạt có độ trễ (Hysteresis)
            if (t > TEMP_THRESHOLD) {
                digitalWrite(PIN_RELAY, HIGH); // Bật quạt
            } else if (t < (TEMP_THRESHOLD - HYSTERESIS)) {
                digitalWrite(PIN_RELAY, LOW);  // Tắt quạt
            }

            // Hiển thị OLED
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0, 0);
            display.print("QUAT: ");
            display.println(digitalRead(PIN_RELAY) ? "DANG BAT" : "DANG TAT");
            
            display.setCursor(0, 30);
            display.setTextSize(2);
            display.print(t, 1);
            display.println(" C");
            display.display();
        }
    }
}