/*
THÔNG TIN NHÓM X
1. Huỳnh Thị Thủy
2. Đào Thị Thùy Dương
3. Hồ Thị Thanh Bình 
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== DHT22 =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_GREEN 15
#define LED_YELLOW 2
#define LED_RED 4

// ===== TIMER =====
unsigned long previousDHT = 0;
unsigned long previousBlink = 0;

const long dhtInterval = 2000;   // đọc DHT mỗi 2s
const long blinkInterval = 300;  // nhấp nháy LED

// ===== BIẾN TRẠNG THÁI =====
float temp = 0;
float hum = 0;
String status = "";
int activeLED = -1;
bool ledState = false;

void updateStatus(float t) {
  if (t < 13) {
    status = "TOO COLD";
    activeLED = LED_GREEN;
  }
  else if (t < 20) {
    status = "COLD";
    activeLED = LED_GREEN;
  }
  else if (t < 25) {
    status = "COOL";
    activeLED = LED_YELLOW;
  }
  else if (t < 30) {
    status = "WARM";
    activeLED = LED_YELLOW;
  }
  else if (t < 35) {
    status = "HOT";
    activeLED = LED_RED;
  }
  else {
    status = "TOO HOT";
    activeLED = LED_RED;
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature");

  display.setTextSize(2);
  display.setCursor(0, 12);
  display.print(temp);
  display.print(" C");

  display.setTextSize(1);
  display.setCursor(0, 38);
  display.print("Humidity: ");
  display.print(hum);
  display.print(" %");

  display.setCursor(0, 54);
  display.print("Status: ");
  display.print(status);

  display.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  dht.begin();
  Wire.begin(13, 12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // ===== ĐỌC DHT ĐỊNH KỲ =====
  if (currentMillis - previousDHT >= dhtInterval) {
    previousDHT = currentMillis;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      temp = t;
      hum = h;
      updateStatus(temp);
      updateOLED();

      Serial.print("Temp: ");
      Serial.print(temp);
      Serial.print("  Hum: ");
      Serial.println(hum);
    }
  }

  // ===== LED NHẤP NHÁY NON-BLOCKING =====
  if (currentMillis - previousBlink >= blinkInterval) {
    previousBlink = currentMillis;

    ledState = !ledState;

    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    if (activeLED != -1 && ledState) {
      digitalWrite(activeLED, HIGH);
    }
  }
}
