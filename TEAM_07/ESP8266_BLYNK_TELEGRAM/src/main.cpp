/*
THÔNG TIN NHÓM 07
1. Đào Thị Thùy Dương
2. ...
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

