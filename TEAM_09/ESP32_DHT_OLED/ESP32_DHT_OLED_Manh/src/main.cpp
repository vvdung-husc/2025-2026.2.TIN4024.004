/*
THÔNG TIN NHÓM 9
1. Cao Huy Minh Quân
2. Quách Đạo Mạnh
3. Nguyễn Văn Quốc
4. Phan Anh Tài
5. Thái Thanh Sơn
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);


#define LED_GREEN 15
#define LED_YELLOW 2
#define LED_RED 4


unsigned long lastBlink = 0;
unsigned long lastDHTRead = 0;

bool ledState = false;

const unsigned long blinkInterval = 1000;   // 1s ON - 1s OFF
const unsigned long dhtInterval   = 2000;   // DHT22 đọc mẫu mỗi 2s


float temp = NAN;
float humi = NAN;
int activeLed = -1;
int lastActiveLed = -1;
String lastStatus = "";


void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  dht.begin();
  Wire.begin(13, 12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  Serial.println("=== SYSTEM START ===");
}


void blinkLed(int ledPin) {
  unsigned long now = millis();

  if (now - lastBlink >= blinkInterval) {
    lastBlink = now;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    
    //In log để xác định số giây led on (1s) - off (1s)
    // Serial.print("[LED] ");
    // Serial.print(ledState ? "ON  " : "OFF ");
    // Serial.print("at ");
    // Serial.print(now / 1000.0);
    // Serial.println(" s");
  }
}


void loop() {
  unsigned long now = millis();

  //DHT22 đọc mẫu mỗi 2s 
  if (now - lastDHTRead >= dhtInterval) {
    lastDHTRead = now;

    temp = dht.readTemperature();
    humi = dht.readHumidity();

    //In log để xác định thời gian đọc mẫu
    Serial.print("[DHT22] Sample at ");
    Serial.print(now / 1000.0);
    Serial.println(" s");

    if (isnan(temp) || isnan(humi)) {
      Serial.println("DHT22 read failed!");
      return;
    }

 //Xác định trạng thái
    if (temp < 13) {
      lastStatus = "TOO COLD";
      activeLed = LED_GREEN;
    } else if (temp < 20) {
      lastStatus = "COLD";
      activeLed = LED_GREEN;
    } else if (temp < 25) {
      lastStatus = "COOL";
      activeLed = LED_YELLOW;
    } else if (temp < 30) {
      lastStatus = "WARM";
      activeLed = LED_YELLOW;
    } else if (temp < 35) {
      lastStatus = "HOT";
      activeLed = LED_RED;
    } else {
      lastStatus = "TOO HOT";
      activeLed = LED_RED;
    }

    if (activeLed != lastActiveLed) {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, LOW);

      ledState = false;
      lastBlink = now;
      lastActiveLed = activeLed;

      // Serial.println("[LED] Change LED → reset blink");
    }

    Serial.print("Status: ");
    Serial.println(lastStatus);
  }

  //Led nhấp nháy
  if (activeLed != -1) {
    blinkLed(activeLed);
  }

  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temp, 1);
  display.print(" C");

  display.setCursor(0, 16);
  display.print("Humi: ");
  display.print(humi, 1);
  display.print(" %");

  display.setCursor(0, 32);
  display.print("Status: ");
  display.print(lastStatus);

  display.display();
}