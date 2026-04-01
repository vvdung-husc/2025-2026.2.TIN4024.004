#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP085 bmp;
#define LED_PIN 23

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("LOI: OLED ko chay!");
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("DANG KHOI DONG...");
  display.display();

  dht.begin();
  if (!bmp.begin()) {
    Serial.println("LOI: Khong thay BMP180!");
    while(1) { 
        digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW); delay(100);
    }
  }
  Serial.println("HE THONG DA SAN SANG!");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  float p = bmp.readPressure() / 100.0F;

  // In ra Terminal
  Serial.print("T: "); Serial.print(t);
  Serial.print(" H: "); Serial.print(h);
  Serial.print(" P: "); Serial.println(p);

  // In ra OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("THONG SO:");
  display.print("Temp: "); display.print(t, 1); display.println(" C");
  display.print("Humi: "); display.print(h, 1); display.println(" %");
  display.print("Pres: "); display.print(p, 1); display.println(" hPa");
  display.display();

  digitalWrite(LED_PIN, LOW);
  delay(2000);
}