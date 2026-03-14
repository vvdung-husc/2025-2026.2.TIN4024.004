#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>


const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define BOTtoken "your_bot_token_here"  
#define CHAT_ID "your_chat_id_here"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int ledPin = 23;
const int motionSensor = 27;

bool ledState = LOW;
volatile bool motionDetected = false;

unsigned long lastTimeBotRan = 0;
int bot_delay = 200;   // kiểm tra telegram nhanh hơn

unsigned long lastMotionMsg = 0;
int motionCooldown = 5000; // tránh spam 5 giây

void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) continue;

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Xin chào " + from_name + "!\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on : Bật sáng đèn\n";
      welcome += "Gửi /led_off : để tắt đèn\n";
      welcome += "Gửi /get_state : để yêu cầu trạng thái đèn hiện tại";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }

    if (text == "/led_off") {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED tắt", "");
    }

    if (text == "/get_state") {
      String msg = (digitalRead(ledPin)) ? "LED is ON" : "LED is OFF";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.print("Connecting Wifi: ");
  WiFi.begin(ssid, password);

  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  if (millis() - lastTimeBotRan > bot_delay) {

    Serial.println("Checking Telegram...");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (numNewMessages) {
      handleNewMessages(numNewMessages);
    }

    lastTimeBotRan = millis();
  }

  if (motionDetected) {

    if (millis() - lastMotionMsg > motionCooldown) {

      Serial.println("Phat hien chuyen dong!");
      bot.sendMessage(CHAT_ID, "Cảnh báo: Phát hiện chuyển động!", "");

      lastMotionMsg = millis();
    }

    motionDetected = false;
  }
}