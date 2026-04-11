#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- CẤU HÌNH WIFI & THINGSPEAK ---
const char* ssid = "Wokwi-GUEST"; // Wi-Fi mặc định của Wokwi (Không cần pass)
const char* password = "";
String serverName = "http://api.thingspeak.com/update";
String apiKey = "3T7EL7AJOHGBNSD1";

// --- CẤU HÌNH PHẦN CỨNG ---
#define PIN_DHT         16  
#define DHT_TYPE        DHT22
#define PIN_RELAY       2   
#define PIN_OLED_SDA    13  
#define PIN_OLED_SCL    12  
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

// --- CẤU HÌNH LOGIC (HYSTERESIS) ---
#define TEMP_ON         30.0 
#define TEMP_OFF        28.5 

DHT dht(PIN_DHT, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long lastReadTime = 0;
unsigned long lastThingSpeakTime = 0;
bool isFanOn = false; 

void setup() {
    Serial.begin(115200);

    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);
    
    dht.begin();

    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("OLED ERROR!"));
        while (true) delay(100);
    }

    // --- KẾT NỐI WIFI ---
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("Connecting Wi-Fi...");
    display.display();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Wi-Fi Connected!");
    display.display();
    delay(1000);
}

void loop() {
    // 1. LOGIC ĐIỀU KHIỂN & OLED (Chạy mỗi 2 giây)
    if (millis() - lastReadTime >= 2000) {
        lastReadTime = millis();
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (isnan(t) || isnan(h)) {
            digitalWrite(PIN_RELAY, LOW);
            isFanOn = false;
            display.clearDisplay();
            display.setCursor(10, 20);
            display.setTextSize(2);
            display.println("SENSOR ERR");
            display.display();
            return; 
        }

        if (t >= TEMP_ON && !isFanOn) {
            digitalWrite(PIN_RELAY, HIGH);
            isFanOn = true;
        } else if (t <= TEMP_OFF && isFanOn) {
            digitalWrite(PIN_RELAY, LOW);
            isFanOn = false;
        }

        // Hiện OLED
        display.clearDisplay();
        display.drawRect(0, 0, 128, 64, WHITE);
        display.setTextSize(1);
        display.setCursor(5, 5);
        display.print("QUAT: ");
        display.println(isFanOn ? "ON" : "OFF");
        display.drawLine(0, 18, 128, 18, WHITE);
        display.setTextSize(2);
        display.setCursor(5, 25);
        display.print(t, 1);
        display.print("C");
        display.setTextSize(1);
        display.setCursor(85, 25);
        display.print("AM DO");
        display.setCursor(85, 40);
        display.print(h, 0);
        display.print("%");
        display.display();

        // 2. LOGIC ĐẨY DATA LÊN THINGSPEAK (Chạy mỗi 15 giây)
        if (WiFi.status() == WL_CONNECTED && millis() - lastThingSpeakTime >= 15000) {
            lastThingSpeakTime = millis();
            HTTPClient http;
            String serverPath = serverName + "?api_key=" + apiKey + "&field1=" + String(t) + "&field2=" + String(h) + "&field3=" + String(isFanOn);
            
            http.begin(serverPath.c_str());
            int httpResponseCode = http.GET();
            
            if (httpResponseCode > 0) {
                Serial.print("ThingSpeak Update Success: ");
                Serial.println(httpResponseCode);
            } else {
                Serial.print("ThingSpeak Error: ");
                Serial.println(httpResponseCode);
            }
            http.end();
        }
    }
}