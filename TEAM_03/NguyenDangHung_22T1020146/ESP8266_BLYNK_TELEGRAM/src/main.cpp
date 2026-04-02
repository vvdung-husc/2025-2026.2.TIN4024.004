#define BLYNK_TEMPLATE_ID "TMPL60lFJ3zny"
#define BLYNK_TEMPLATE_NAME "ESP8266BlynkTelegram"
#define BLYNK_AUTH_TOKEN "z2KHpYTqG5sEHpc-IDZi4NlBr2f3dE44"

// ===== WIFI =====
#include <ESP8266WiFi.h>
char ssid[] = "hung";
char pass[] = "11111111";

// ===== TELEGRAM =====
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#define BOT_TOKEN "8787158074:AAEouuAU8HpwWoRHVvVVyGU74gEMi4GV-XU"
#define CHAT_ID "-5199524596"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ===== BLYNK =====
#include <BlynkSimpleEsp8266.h>
BlynkTimer timer;

// ===== SENSOR =====
#include <DHT.h>
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== OLED =====
#include <Wire.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &Wire);

// ===== HARDWARE =====
#define LED_PIN D5
#define PIR_PIN D6
#define RELAY1 D1
#define RELAY2 D2
#define GAS_PIN A0

bool ledState = false;
float temp = 0;
float hum = 0;
int gas = 0;
int gas_ppm = 0;
int pirState = 0;

unsigned long startTime;
bool gasAlertSent = false;

// ===== BLYNK CONTROL =====
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

// ===== READ SENSOR =====
void readSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) temp = t;
  if (!isnan(h)) hum = h;

  gas = analogRead(GAS_PIN);
  gas_ppm = map(gas, 0, 1023, 0, 1000);

  pirState = digitalRead(PIR_PIN);

  // Gửi Blynk
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V4, gas_ppm);
  Blynk.virtualWrite(V5, pirState);

  Serial.printf("T: %.1f | H: %.1f | Gas: %d | PIR: %d\n",
                temp, hum, gas_ppm, pirState);

  // 🚨 Cảnh báo GAS
  if (gas_ppm > 700 && !gasAlertSent) {
    bot.sendMessage(CHAT_ID,
      "🚨 CANH BAO GAS!\nPPM: " + String(gas_ppm),
      "");
    gasAlertSent = true;
  }

  if (gas_ppm < 500) {
    gasAlertSent = false;
  }
}

// ===== OLED =====
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Uptime: ");
  display.println((millis() - startTime) / 1000);

  display.print("LED: ");
  display.println(ledState ? "ON" : "OFF");

  display.print("Temp: ");
  display.println(temp);

  display.print("Hum: ");
  display.println(hum);

  display.print("Gas: ");
  display.print(gas_ppm);
  display.println(" ppm");

  display.print("PIR: ");
  display.println(pirState);

  display.display();

  Blynk.virtualWrite(V0, (millis() - startTime) / 1000);
}

// ===== TELEGRAM =====
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;

      // LED
      if (text == "/led_on") {
        digitalWrite(LED_PIN, HIGH);
        ledState = true;
        Blynk.virtualWrite(V1, 1);
        bot.sendMessage(CHAT_ID, "LED ON", "");
      }

      if (text == "/led_off") {
        digitalWrite(LED_PIN, LOW);
        ledState = false;
        Blynk.virtualWrite(V1, 0);
        bot.sendMessage(CHAT_ID, "LED OFF", "");
      }

      if (text == "/led_status") {
        bot.sendMessage(CHAT_ID,
          ledState ? "LED ON" : "LED OFF", "");
      }

      // Sensor
      if (text == "/get_weather") {
        bot.sendMessage(CHAT_ID,
          "Temp: " + String(temp) +
          "\nHum: " + String(hum),
          "");
      }

      if (text == "/gas") {
        bot.sendMessage(CHAT_ID,
          "Gas: " + String(gas_ppm) + " ppm",
          "");
      }

      if (text == "/motion") {
        bot.sendMessage(CHAT_ID,
          pirState ? "Co chuyen dong!" : "Khong co chuyen dong",
          "");
      }

      if (text == "/all") {
        bot.sendMessage(CHAT_ID,
          "📊 DATA:\nTemp: " + String(temp) +
          "\nHum: " + String(hum) +
          "\nGas: " + String(gas_ppm) +
          "\nPIR: " + String(pirState),
          "");
      }
    }

    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  dht.begin();

  // OLED I2C ESP8266
  Wire.begin(D2, D1);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Telegram HTTPS
  client.setInsecure();

  // Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  startTime = millis();

  // Timer
  timer.setInterval(2000L, readSensor);
  timer.setInterval(1000L, updateOLED);
  timer.setInterval(1000L, handleTelegram);
}

// ===== LOOP =====
void loop() {
  Blynk.run();
  timer.run();
}