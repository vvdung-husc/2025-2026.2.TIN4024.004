/*
THÔNG TIN NHÓM
1. Phan Hữu Tuấn Kiệt
2. Nguyễn Đại Quý
3. Nguyễn Đình Hoàng
4.
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= DHT22 =================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ================= LED ==================
#define LED_GREEN   25
#define LED_YELLOW  26
#define LED_RED     27

// ================= TIMER =================
unsigned long lastBlinkMillis = 0;
unsigned long lastReadMillis  = 0;
bool ledState = false;
int activeLed = -1;

const long blinkInterval = 300;   // ms
const long readInterval  = 2000;  // ms

// =======================================

void turnOffAllLED() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
}

// =======================================

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  Wire.begin(21,22);   // SDA, SCL
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED FAIL");
    while(true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

// =======================================

void loop() {

  // ===== ĐỌC CẢM BIẾN =====
  if (millis() - lastReadMillis >= readInterval) {
    lastReadMillis = millis();

    float temp = dht.readTemperature();
    float humi = dht.readHumidity();

    if (isnan(temp) || isnan(humi)) {
      Serial.println("DHT ERROR");
      return;
    }

    Serial.print("Temp: ");
    Serial.println(temp);

    String statusText = "";

    // ===== PHÂN NGƯỠNG =====
    if (temp < 13) {
      statusText = "TOO COLD";
      activeLed = LED_GREEN;
    }
    else if (temp < 20) {
      statusText = "COLD";
      activeLed = LED_GREEN;
    }
    else if (temp < 25) {
      statusText = "COOL";
      activeLed = LED_YELLOW;
    }
    else if (temp < 30) {
      statusText = "WARM";
      activeLed = LED_YELLOW;
    }
    else if (temp < 35) {
      statusText = "HOT";
      activeLed = LED_RED;
    }
    else {
      statusText = "TOO HOT";
      activeLed = LED_RED;
    }

    // ===== OLED =====
    display.clearDisplay();

    display.setCursor(0,0);
    display.print("Temp: ");
    display.print(temp);
    display.println(" C");

    display.setCursor(0,15);
    display.print("Humi: ");
    display.print(humi);
    display.println(" %");

    display.setCursor(0,35);
    display.print("Status:");
    display.setCursor(0,50);
    display.print(statusText);

    display.display();
  }

  // ===== LED BLINK =====
  if (activeLed != -1) {
    if (millis() - lastBlinkMillis >= blinkInterval) {
      lastBlinkMillis = millis();
      ledState = !ledState;

      turnOffAllLED();

      if (ledState) {
        digitalWrite(activeLed, HIGH);
      }
    }
  }
}
