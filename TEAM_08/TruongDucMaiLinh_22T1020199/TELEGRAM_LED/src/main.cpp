#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
//#include <TelegramCertificate.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

//TELEGRAM 
#define BOTtoken "8691422205:AAEfoasr7WCdGzG8qnMweCNPBHb_bsV3d0Q"
#define GROUP_ID "-5185949617"

//CLIENT
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//PIR + LED
const int motionSensor = 27;
//bool motionDetected = false;
int led = 4;

//TIMER
unsigned long lastTimeBotRan;
int botRequestDelay = 1000; // đọc Telegram mỗi 1s

bool lastMotionState = LOW;

void setup() {
  Serial.begin(115200);

  pinMode(motionSensor, INPUT);
  pinMode(led, OUTPUT);

  // WIFI
  WiFi.begin(ssid, password);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  
  client.setInsecure();

  bot.sendMessage(GROUP_ID, "ESP32 started!", "");
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    // chỉ nhận đúng group
    if (chat_id != GROUP_ID) continue;

    if (text == "/on") {
      digitalWrite(led, HIGH);
      bot.sendMessage(GROUP_ID, "💡 LED ON", "");
    }

    if (text == "/off") {
      digitalWrite(led, LOW);
      bot.sendMessage(GROUP_ID, "💡 LED OFF", "");
    }
  }
}

void loop() {

  // đọc Telegram không block
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }

  // PIR không dùng interrupt nữa (mượt hơn Wokwi)
  int motionState = digitalRead(motionSensor);

  if (motionState == HIGH && lastMotionState == LOW) {
    Serial.println("Motion detected!");

    digitalWrite(led, HIGH);
    bot.sendMessage(GROUP_ID, "🚨 Motion detected!", "");
  }

  if (motionState == LOW && lastMotionState == HIGH) {
    digitalWrite(led, LOW);
  }

  lastMotionState = motionState;
}