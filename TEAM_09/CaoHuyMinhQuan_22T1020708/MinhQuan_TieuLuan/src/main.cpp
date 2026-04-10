#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <UniversalTelegramBot.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define MAX30100_ADDR 0x57

// ===== WiFi + Telegram =====
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define BOTtoken "8646963702:AAEih2EXlOAOgdtGmffWgIweFjO0-QDNxDU"
#define CHAT_ID "7984027108"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);

// ===== OLED =====
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Ngưỡng nhịp tim =====
const uint8_t HEART_RATE_LOW = 60;
const uint8_t HEART_RATE_HIGH = 100;

// ===== Gửi Telegram định kỳ =====
unsigned long lastTelegramSend = 0;
const unsigned long telegramInterval = 10000; // gửi mỗi 10 giây

uint8_t readReg(uint8_t reg) {
  Wire.beginTransmission(MAX30100_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);

  Wire.requestFrom(MAX30100_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

void connectWiFi() {
  Serial.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

String getHeartRateStatus(uint8_t bpm) {
  if (bpm > HEART_RATE_HIGH) {
    return "HIGH";
  } else if (bpm < HEART_RATE_LOW) {
    return "LOW";
  }
  return "NORMAL";
}

void sendTelegramMessage(uint8_t bpm) {
  String status = getHeartRateStatus(bpm);
  String message;

  if (status == "HIGH") {
    message = "CANH BAO NHIP TIM CAO\n";
    message += "Nhip tim hien tai: " + String(bpm) + " bpm";
  } else if (status == "LOW") {
    message = "CANH BAO NHIP TIM THAP\n";
    message += "Nhip tim hien tai: " + String(bpm) + " bpm";
  } else {
    message = "Du lieu nhip tim binh thuong\n";
    message += "Nhip tim hien tai: " + String(bpm) + " bpm";
  }

  bool ok = bot.sendMessage(CHAT_ID, message, "");
  if (ok) {
    Serial.println("Telegram sent");
  } else {
    Serial.println("Telegram send failed");
  }
}

void showHeartRate(uint8_t bpm) {
  String status = getHeartRateStatus(bpm);
  String warningText = "Normal";

  if (status == "HIGH") {
    warningText = "Warning: HR High!";
  } else if (status == "LOW") {
    warningText = "Warning: HR Low!";
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Heart Rate Monitor");

  display.setTextSize(2);
  display.setCursor(0, 18);
  display.print("HR: ");
  display.print(bpm);

  display.setTextSize(1);
  display.setCursor(0, 50);
  display.println(warningText);

  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Heart Rate System");
  display.println("Starting...");
  display.display();

  connectWiFi();
  secured_client.setInsecure();

  Serial.println("BOOT OK");
  delay(1000);
}

void loop() {
  uint8_t bpm = readReg(0x00);

  Serial.print("BPM: ");
  Serial.println(bpm);

  showHeartRate(bpm);

  if (millis() - lastTelegramSend > telegramInterval) {
    lastTelegramSend = millis();
    sendTelegramMessage(bpm);
  }

  delay(500);
}