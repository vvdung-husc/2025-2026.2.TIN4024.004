/*
THÔNG TIN NHÓM 05
1. Trương Đức Long 
2. Trần Xuân Tấn Vinh
3. Phan Nguyễn Anh Nhật
4. Võ Hoàng Minh Phước
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define PIN_DHT 16
#define DHTTYPE DHT22
#define PIN_LED_RED 4
#define PIN_LED_GREEN 15
#define PIN_LED_YELLOW 2
#define OLED_SDA 13
#define OLED_SCL 12

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DHT dht(PIN_DHT, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long blinkTimer = 0;
bool ledStatus = false;

void setup() {
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);

  Wire.begin(OLED_SDA, OLED_SCL);
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) return;

  String statusMsg = "";
  int activeLed = -1;

  if (temp < 13) {
    statusMsg = "TOO COLD";
    activeLed = PIN_LED_GREEN;
  } else if (temp < 20) {
    statusMsg = "COLD";
    activeLed = PIN_LED_GREEN;
  } else if (temp < 25) {
    statusMsg = "COOL";
    activeLed = PIN_LED_YELLOW;
  } else if (temp < 30) {
    statusMsg = "WARM";
    activeLed = PIN_LED_YELLOW;
  } else if (temp < 35) {
    statusMsg = "HOT";
    activeLed = PIN_LED_RED;
  } else {
    statusMsg = "TOO HOT";
    activeLed = PIN_LED_RED;
  }

  if (millis() - blinkTimer >= 500) {
    blinkTimer = millis();
    ledStatus = !ledStatus;
    
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    
    if (activeLed != -1) {
      digitalWrite(activeLed, ledStatus);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: "); display.print(temp, 1); display.println(" C");
  display.print("Hum:  "); display.print(hum, 1); display.println(" %");
  display.println("--------------------");
  
  display.setTextSize(2);
  display.setCursor(0, 35);
  display.println(statusMsg);
  display.display();
}