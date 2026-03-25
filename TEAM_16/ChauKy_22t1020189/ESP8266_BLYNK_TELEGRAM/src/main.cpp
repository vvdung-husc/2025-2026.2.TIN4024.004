/*
*/

#define BLYNK_TEMPLATE_ID   "TMPL6haiWo-5w"
#define BLYNK_TEMPLATE_NAME "ESP8266BlynkTelegram"
#define BLYNK_AUTH_TOKEN    "hPZd_EqsL49KSzap970iPuu5L7ABRjq1"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <U8g2lib.h>

// ================= WIFI =================
const char* ssid = "CNTT-MMT";
const char* password = "13572468";

// ================= TELEGRAM =================
#define BOT_TOKEN "8596904885:AAEn5nzGt3JO0Ft2DH6bSj7uN_Iqik5GFQw"
#define CHAT_ID   "-5255423534"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ================= OLED =================
// SSD1306 128x64 I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ================= PHẦN CỨNG =================
#define DHT_PIN   D3
#define DHT_TYPE  DHT11
#define LED_RED   D6
#define LED_BLUE  D4
#define MQ2_PIN   A0

DHT dht(DHT_PIN, DHT_TYPE);

BlynkTimer timer;

// ================= BIẾN =================
float temperature = 0;
float humidity    = 0;
int gasValue      = 0;

bool ledState = false;

float lastTemp = -999;
float lastHum  = -999;

bool gasAlertSent = false;

// ================= BLYNK =================
BLYNK_WRITE(V0) {
  ledState = param.asInt();
  digitalWrite(LED_RED,  ledState);
  digitalWrite(LED_BLUE, ledState);
}

// ================= OLED =================
void displayOLED() {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(0, 10, "ESP8266 MONITOR");

  char buf[32];

  sprintf(buf, "Temp: %.1f C", temperature);
  u8g2.drawStr(0, 25, buf);

  sprintf(buf, "Hum : %.1f %%", humidity);
  u8g2.drawStr(0, 40, buf);

  sprintf(buf, "Gas : %d", gasValue);
  u8g2.drawStr(0, 55, buf);

  u8g2.sendBuffer();
}

// ================= ĐỌC CẢM BIẾN =================
void sendSensorData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int   g = analogRead(MQ2_PIN);

  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity    = h;
  gasValue = g;

  // gửi Blynk
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V3, gasValue);

  Serial.printf("T: %.1f | H: %.1f | Gas: %d\n", temperature, humidity, gasValue);

  // gửi Telegram khi thay đổi
  if (abs(temperature - lastTemp) > 1 || abs(humidity - lastHum) > 2) {
    String msg = "📊 Update:\n";
    msg += "🌡 " + String(temperature,1) + " C\n";
    msg += "💧 " + String(humidity,1) + " %";
    bot.sendMessage(CHAT_ID, msg, "");

    lastTemp = temperature;
    lastHum  = humidity;
  }

  // 🚨 cảnh báo gas
  if (gasValue > 800 && !gasAlertSent) {
    bot.sendMessage(CHAT_ID, "🚨 GAS CAO !!!", "");
    gasAlertSent = true;
  }
  if (gasValue < 600) {
    gasAlertSent = false;
  }

  // 👉 update OLED
  displayOLED();
}

// ================= TELEGRAM =================
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) continue;

    if (text == "/start") {
      bot.sendMessage(chat_id,
        "/led_on\n/led_off\n/status\n/weather", "");
    }
    else if (text == "/led_on") {
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_BLUE, HIGH);
      Blynk.virtualWrite(V0, 1);
    }
    else if (text == "/led_off") {
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_BLUE, LOW);
      Blynk.virtualWrite(V0, 0);
    }
    else if (text == "/status") {
      bot.sendMessage(chat_id, "Gas: " + String(gasValue), "");
    }
    else if (text == "/weather") {
      String msg = "Temp: " + String(temperature) + " C\n";
      msg += "Hum: " + String(humidity) + " %";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  dht.begin();

  // OLED
  u8g2.begin();

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK");

  client.setInsecure();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  timer.setInterval(5000L, sendSensorData);
  timer.setInterval(3000L, handleTelegram);
}

// ================= LOOP =================
void loop() {
  Blynk.run();
  timer.run();
}