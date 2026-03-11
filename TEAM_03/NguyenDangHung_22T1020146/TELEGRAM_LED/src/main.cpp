#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram
#define BOTtoken "8634532983:AAFq50hnfy1zCuBbm1PZdX1boO9Avf4G5uw"
#define GROUP_ID "-5241642816"

// PIR sensor
const int motionSensor = 27;

// LED (diagram nối vào GPIO23)
const int ledPin = 23;

bool motionDetected = false;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// thời gian kiểm tra tin nhắn Telegram
unsigned long lastTimeBotRan = 0;
const int botRequestDelay = 1000;

// format string
String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);

  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);

  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);

  String s = buff;
  free(buff);

  return s;
}

// interrupt PIR
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// xử lý lệnh Telegram
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    Serial.println(text);

    if (text == "/start") {
      String welcome = "Xin chào.\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "/led_on : bật đèn\n";
      welcome += "/led_off : tắt đèn\n";
      welcome += "/get_state : xem trạng thái đèn\n";

      bot.sendMessage(chat_id, welcome);
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "LED is ON");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "LED is OFF");
    }

    if (text == "/get_state") {

      if (digitalRead(ledPin)) {
        bot.sendMessage(chat_id, "LED đã sáng");
      } 
      else {
        bot.sendMessage(chat_id, "LED đã tắt");
      }
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");

  bot.sendMessage(GROUP_ID, "ESP32 Telegram Bot Started");
}

void loop() {

  static unsigned int count_ = 0;

  // PIR phát hiện chuyển động
  if (motionDetected) {

    count_++;

    Serial.print(count_);
    Serial.println(". MOTION DETECTED");

    String msg = StringFormat("%u => Motion detected!", count_);

    bot.sendMessage(GROUP_ID, msg);

    motionDetected = false;
  }

  // đọc lệnh Telegram
  if (millis() - lastTimeBotRan > botRequestDelay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}