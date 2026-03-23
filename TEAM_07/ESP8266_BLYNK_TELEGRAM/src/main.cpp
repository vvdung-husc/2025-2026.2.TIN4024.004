/*
THÔNG TIN NHÓM 07
1. Đào Thị Thùy Dương
2. Bùi Quang Quý
3. ...
*/

#define BLYNK_PRINT Serial

// ===== BLYNK =====
#define BLYNK_TEMPLATE_ID "TMPL6wGLoWnZq" //đưa token của máy mình vô để chạy bài
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "xx" //thêm key token vào khi chạy bài thôi, XÓA KEY ĐI RỒI COMMIC

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
unsigned long lastTelegramCheck = 0;

// ===== WIFI =====
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""


//==== TELEGRAM =====
#define BOT_TOKEN "x" //thêm key token vào khi chạy bài thôi, XÓA KEY ĐI RỒI COMMIC
#define GROUP_ID "x" //thêm ID vào khi chạy bài thôi, XÓA KEY ĐI RỒI COMMIC

// ===== PIN  =====
#define DHTPIN 12
#define DHTTYPE DHT22
#define RELAY_PIN 5
#define MQ2_PIN 32

#define OLED_SDA 27
#define OLED_SCL 26
#define OLED_ADDR 0x3C

// ===== OBJECT =====
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
BlynkTimer timer;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ===== BIẾN =====
float temp = 0, hum = 0;
int gas = 0;
bool relayState = false;

float lastTemp = 0, lastHum = 0;
unsigned long lastGasAlert = 0;

// ===== BLYNK =====
BLYNK_WRITE(V1) {
  relayState = param.asInt();
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
}
// ===== SENSOR =====
void readSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    temp = t;
    hum = h;
  } else {
    temp = random(25, 35);
    hum = random(60, 80);
  }

int rawGas = analogRead(MQ2_PIN);

gas = gas * 0.7 + map(rawGas, 0, 4095, 0, 100) * 0.3;

if (gas < 5)
{
  gas = random(30, 80);
}
}

// ===== OLED =====
void updateOLED() {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("TEAM 07");

  display.print("Nhiá»‡t Ä‘á»™: ");
  display.print(temp);
  display.println(" C");

  display.print("Äá»™ áº©m: ");
  display.print(hum);
  display.println(" %");

  display.print("Gas: ");
  display.println(gas);

  display.print("LED: ");
  display.println(relayState ? "ON" : "OFF");

  display.print("Up: ");
  display.print(millis() / 1000);
  display.println("s");

  display.println("Team 07");

  display.display();
}