#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram
#define BOTtoken "your-bot-token"
#define GROUP_ID "your-group-id"

// Hardware (theo diagram)
#define motionSensor 27
#define LED_PIN 23

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

bool motionDetected = false;
bool ledState = false;

unsigned long lastTimeBotRan;
int botRequestDelay = 1000;

uint count_ = 0;


// PIR interrupt
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}


// xử lý tin nhắn telegram
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (text == "/start") {
      String welcome = "Xin chào, Long\n";
      welcome += "Sử dụng các lệnh sau để điều khiển LED\n\n";
      welcome += "/led_on : Bật LED\n";
      welcome += "/led_off : Tắt LED\n";
      welcome += "/get_state : Trạng thái LED hiện tại\n";
      

      bot.sendMessage(chat_id, welcome, "");
    }

    // bật LED
    if (text == "/led_on") {

      digitalWrite(LED_PIN, HIGH);
      ledState = true;

      bot.sendMessage(chat_id, "LED đã được bật", "");
    }

    // tắt LED
    if (text == "/led_off") {

      digitalWrite(LED_PIN, LOW);
      ledState = false;

      bot.sendMessage(chat_id, "LED đã được tắt", "");
    }

    // trạng thái LED
    if (text == "/get_state") {

      if (ledState)
        bot.sendMessage(chat_id, "LED is ON", "");
      else
        bot.sendMessage(chat_id, "LED is OFF", "");
    }
  }
}


void setup() {

  Serial.begin(115200);

  pinMode(motionSensor, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");
    delay(100);
  }

  Serial.println("\nWiFi connected");

  bot.sendMessage(GROUP_ID, "ESP32 System Started");
}


void loop() {

  // PIR phát hiện chuyển động
  if (motionDetected) {

    count_++;

    Serial.println("Motion detected!");

    bot.sendMessage(GROUP_ID, "Motion detected!", "");

    motionDetected = false;
  }

  // đọc tin nhắn telegram
  if (millis() > lastTimeBotRan + botRequestDelay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}