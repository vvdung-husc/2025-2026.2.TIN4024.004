/*
  THÔNG TIN NHÓM 8
  1. Trương Đức Mai Linh
  2. Zơrâm Nhỏ
  3. Phạm Hữu Anh Kiệt
*/



#define BLYNK_PRINT Serial

// ===== BLYNK =====
#define BLYNK_TEMPLATE_ID "TMPL69uPnysdL"
#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "6Wf_PMqvwPnn3yFGwGGd04Y2DojetEHD"


// ===== WIFI =====
//#define WIFI_SSID "Wokwi-GUEST"
//#define WIFI_PASSWORD ""
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* BOTtoken = "8594921331:AAFZdT4p95pq-c-MCCL_1ccx20AvWIuJVyI";
const char* CHAT_ID = "-5294764327";
//#define BOTtoken "8594921331:AAFZdT4p95pq-c-MCCL_1ccx20AvWIuJVyI"
//#define CHAT_ID "-5294764327"


// ===== LIB =====
#include <WiFi.h>
#include <Wire.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== OLED =====
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ===== SENSOR =====
DHT dht(12, DHT22);
#define GAS_PIN 32
#define LED_PIN 5

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
BlynkTimer timer;

// ===== BIẾN =====
float temp, hum, gasPPM;

// ===== ĐỌC SENSOR =====
void readSensor() {
  temp = dht.readTemperature();
  hum  = dht.readHumidity();

  int gas = analogRead(GAS_PIN);
  gasPPM = (gas / 4095.0) * 1000;

  // Blynk
  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, hum);
  Blynk.virtualWrite(V3, gasPPM);

  // Telegram cảnh báo gas
  if (gasPPM > 900) {
    bot.sendMessage(CHAT_ID, "⚠️ Gas cao: " + String(gasPPM), "");
  }

  Serial.printf("T: %.1f | H: %.1f | Gas: %.0f\n", temp, hum, gasPPM);
}

// ===== OLED =====
void showOLED() {
  display.clearDisplay();
  display.setCursor(0,0);

  display.printf("Temp: %.1f C\n", temp);
  display.printf("Hum : %.1f %%\n", hum);
  display.printf("Gas : %.0f ppm\n", gasPPM);

  display.display();
}

// ===== TELEGRAM =====
void handleTelegram() {
  int n = bot.getUpdates(bot.last_message_received + 1);

  while (n) {
    for (int i = 0; i < n; i++) {
      String text = bot.messages[i].text;

      if (text == "/on") {
        digitalWrite(LED_PIN, HIGH);
        bot.sendMessage(CHAT_ID, "LED ON", "");
      }
      else if (text == "/off") {
        digitalWrite(LED_PIN, LOW);
        bot.sendMessage(CHAT_ID, "LED OFF", "");
      }
    }
    n = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ===== BLYNK =====
BLYNK_WRITE(V0) {
  digitalWrite(LED_PIN, param.asInt());
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  Wire.begin(27, 26);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  dht.begin();

  client.setInsecure();
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  timer.setInterval(2000, readSensor);
  timer.setInterval(2000, showOLED);
  timer.setInterval(1500, handleTelegram);
}

// ===== LOOP =====
void loop() {
  Blynk.run();
  timer.run();
}