#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram
#define BOTtoken "8785416571:AAG4ezhAd6Q5qYrpY0gqQHZMOPFszoCXJbk"
#define GROUP_ID "5115367463"

// Pin
const int motionSensor = 27;
const int ledPin = 23;

// Biến
volatile bool motionDetected = false;
unsigned long lastTimeBotRan = 0;
const unsigned long delayBetweenChecks = 1000;

// Client
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ===== INTERRUPT =====
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// ===== HANDLE TELEGRAM =====
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = bot.messages[i].chat_id;
    if (chat_id != GROUP_ID) continue;

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Xin chào " + from_name + "\n";
      welcome += "/led_on - Bật LED\n";
      welcome += "/led_off - Tắt LED\n";
      welcome += "/get_state - Xem trạng thái";

      bot.sendMessage(chat_id, welcome, "");
    }

    else if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "LED ON", "");
    }

    else if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "LED OFF", "");
    }

    else if (text == "/get_state") {
      String state = digitalRead(ledPin) ? "ON" : "OFF";
      bot.sendMessage(chat_id, "LED is " + state, "");
    }
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(motionSensor, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");

  // 🔥 FIX SSL + DNS (QUAN TRỌNG)
  client.setInsecure(); // bỏ certificate (ổn định hơn)

  // Gửi tin nhắn test
  if (bot.sendMessage(GROUP_ID, "✅ Hệ thống đã sẵn sàng!", "")) {
    Serial.println("Telegram OK");
  } else {
    Serial.println("Telegram lỗi!");
  }
}

// ===== LOOP =====
void loop() {

  // PIR detect
  if (motionDetected) {
    motionDetected = false;

    Serial.println("MOTION DETECTED!");

    bot.sendMessage(GROUP_ID, "🚨 Có chuyển động!", "");
  }

  // Telegram check
  if (millis() - lastTimeBotRan > delayBetweenChecks) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}