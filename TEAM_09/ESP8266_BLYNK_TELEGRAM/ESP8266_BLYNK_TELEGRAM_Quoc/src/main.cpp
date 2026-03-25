/*
THÔNG TIN NHÓM 09.004
1.Cao Huy Minh Quân
2.Nguyễn Văn Quốc - Telegram : Quốc Nguyễn
3.
4.
5.

*/


// #define BLYNK_TEMPLATE_ID "TMPL6kETqhf8m"
// #define BLYNK_TEMPLATE_NAME "ESP8266"
// #define BLYNK_AUTH_TOKEN "sU-bAXfAgqGwlxgvZCk-Ks4CKYPQcUL7"

#define BLYNK_TEMPLATE_ID "TMPL6JTM99Cmp"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "werL4X6y8Z7t94PnEcC0SjoCpGDgfeH2"

#include <Arduino.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ===== TELEGRAM =====
#define BOT_TOKEN "8785356162:AAFWBQCwHi-_RW5znQfCWVwFHiM1JnTqGUc"
#define CHAT_ID   "6711911568"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);



char ssid[] = "Wokwi-GUEST";
char pass[] = "";
//PIN
#define DHTPIN D3
#define DHTTYPE DHT11
#define LED_PIN D6

//BLYNKPIN
#define VPIN_UPTIME V0
#define VPIN_TEMP V1
#define VPIN_HUM V2
#define VPIN_LED V3
#define VPIN_TEAM V4

DHT dht(DHTPIN, DHTTYPE);

// ===== TIMER =====
unsigned long tSensor = 0;
unsigned long tUptime = 0;
unsigned long tTeam   = 0;

const long intervalSensor = 2000;
const long intervalUptime = 1000;
const long intervalTeam   = 5000;

unsigned long tTelegram = 0;
const long intervalTelegram = 2000;

float lastTemp = 0;
float lastHum = 0;
// ===== DATA =====
float temp = 0;
float hum = 0;
int gas = 0;

// ===== LED CONTROL =====
BLYNK_WRITE(VPIN_LED)
{
  int value = param.asInt();
  digitalWrite(LED_PIN, value);
}

void handleTelegram()
{
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  for (int i = 0; i < numNewMessages; i++)
  {
    String text = bot.messages[i].text;

    // ===== START MENU =====
    if (text == "/start")
    {
      String msg = "IoT Bot Ready\n\n";
      msg += "Chọn lệnh bên dưới:\n";
      msg += "/led_on - Bat LED\n";
      msg += "/led_off - Tat LED\n";
      msg += "/led_status - Trang thai LED\n";
      msg += "/get_weather - Xem nhiet do, do am";
      bot.sendMessage(CHAT_ID, msg, "");
    }

    else if (text == "/led_on")
    {
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(CHAT_ID, "LED ON", "");
    }

    else if (text == "/led_off")
    {
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(CHAT_ID, "LED OFF", "");
    }

    else if (text == "/led_status")
    {
      String msg = digitalRead(LED_PIN) ? "LED ON" : "LED OFF";
      bot.sendMessage(CHAT_ID, msg, "");
    }

    else if (text == "/get_weather")
    {
      String msg = "Temp: " + String(temp) + " C\n";
      msg += "Hum: " + String(hum) + " %";

      bot.sendMessage(CHAT_ID, msg, "");
    }
  }
}

// ===== SETUP =====
void setup()
{
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  randomSeed(analogRead(A0));

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  client.setInsecure(); 
  Serial.println("Blynk Ready");
}

void loop()
{
  Blynk.run();

  unsigned long now = millis();

  // ===== SENSOR =====
  if (now - tSensor >= intervalSensor)
  {
    tSensor = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) temp = t;
    if (!isnan(h)) hum = h;

    Blynk.virtualWrite(VPIN_TEMP, temp);
    Blynk.virtualWrite(VPIN_HUM, hum);

    // ===== TELEGRAM AUTO SEND =====
    if (abs(temp - lastTemp) > 1 || abs(hum - lastHum) > 3)
    {
      String msg = "Update:\nTemp: " + String(temp) + " C\n";
      msg += "Hum: " + String(hum) + " %";

      bot.sendMessage(CHAT_ID, msg, "");

      lastTemp = temp;
      lastHum = hum;
    }
  }

  // ===== UPTIME =====
  if (now - tUptime >= intervalUptime)
  {
    tUptime = now;
    int uptime = millis() / 1000;
    Blynk.virtualWrite(VPIN_UPTIME, uptime);
  }

  // ===== TEAM NAME =====
  if (now - tTeam >= intervalTeam)
  {
    tTeam = now;
    Blynk.virtualWrite(VPIN_TEAM, "IoT - Team 09.004");
  }

  // ===== TELEGRAM =====
  if (now - tTelegram >= intervalTelegram)
  {
    tTelegram = now;
    handleTelegram();
  }
}