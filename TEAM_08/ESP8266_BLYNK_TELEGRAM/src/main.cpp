	/*
	THÔNG TIN NHÓM 8
	1. Trương Đức Mai Linh
	2. Zơrâm Nhỏ
	
	*/


// ================= BLYNK =================
#define BLYNK_TEMPLATE_ID "TMPL69uPnysdL"
#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "6Wf_PMqvwPnn3yFGwGGd04Y2DojetEHD"   

// ================= WIFI =================
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// ================= LIB =================
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>


// ================= TELEGRAM =================
#define BOT_TOKEN "8594921331:AAFZdT4p95pq-c-MCCL_1ccx20AvWIuJVyI"
#define CHAT_ID "-5294764327"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ================= DHT =================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ================= LED =================
#define LED_PIN 2
bool ledState = false;

// ================= TIMER =================
BlynkTimer timer;

float temp = 0;
float hum = 0;

// ================= BLYNK WRITE =================
BLYNK_WRITE(V1) {
  int value = param.asInt();
  digitalWrite(LED_PIN, value);
  ledState = value;
}

// ================= TELEGRAM =================
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) continue;

    if (text == "/start") {
      bot.sendMessage(chat_id,
        "Commands:\n"
        "/led_on\n"
        "/led_off\n"
        "/led_status\n"
        "/get_weather", "");
    }

    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
      Blynk.virtualWrite(V1, 1);
      bot.sendMessage(chat_id, "LED ON", "");
    }

    if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      ledState = false;
      Blynk.virtualWrite(V1, 0);
      bot.sendMessage(chat_id, "LED OFF", "");
    }

    if (text == "/led_status") {
      bot.sendMessage(chat_id,
        ledState ? "LED ON" : "LED OFF", "");
    }

    if (text == "/get_weather") {
      String msg = "🌡 Data:\n";
      msg += "Temp: " + String(temp) + "°C\n";
      msg += "Hum: " + String(hum) + "%";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// ================= SENSOR + BLYNK =================
void sendSensor() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();

  // Gửi Blynk
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);

  // Uptime
  Blynk.virtualWrite(V0, millis()/1000);

  // Gửi Telegram khi thay đổi
  String msg = "📊 Update:\n";
  msg += "Temp: " + String(temp) + "°C\n";
  msg += "Hum: " + String(hum) + "%";

  bot.sendMessage(CHAT_ID, msg, "");
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  dht.begin();

  timer.setInterval(3000L, sendSensor);
}

// ================= LOOP =================
void loop() {
  Blynk.run();
  timer.run();

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}