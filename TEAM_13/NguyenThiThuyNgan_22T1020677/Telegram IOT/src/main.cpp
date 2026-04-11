#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi (Wokwi dùng cái này)
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// 👉 THAY TOKEN MỚI CỦA BẠN VÀO ĐÂY
#define BOTtoken "8661133707:AAEW-IJx1myEuqmsMJvpZxjlAOWalr41uIQ"

// 👉 CHAT ID CỦA BẠN
#define CHAT_ID "8726144638"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan = 0;
int bot_delay = 1000;

const int ledPin = 23;
bool ledState = LOW;


// ================== XỬ LÝ TIN NHẮN ==================
void handleNewMessages(int numNewMessages) {

  Serial.println("Co tin nhan moi");

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);

    // kiểm tra đúng người dùng
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    // START
    if (text == "/start") {
      String welcome = "Welcome " + from_name + "\n";
      welcome += "/led2_on : Bat LED\n";
      welcome += "/led2_off : Tat LED\n";
      welcome += "/get_state : Trang thai LED\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    // BẬT LED
    if (text == "/led2_on") {
      digitalWrite(ledPin, HIGH);
      ledState = HIGH;
      bot.sendMessage(chat_id, "LED ON", "");
    }

    // TẮT LED
    if (text == "/led2_off") {
      digitalWrite(ledPin, LOW);
      ledState = LOW;
      bot.sendMessage(chat_id, "LED OFF", "");
    }

    // TRẠNG THÁI
    if (text == "/get_state") {
      if (ledState) {
        bot.sendMessage(chat_id, "LED dang BAT", "");
      } else {
        bot.sendMessage(chat_id, "LED dang TAT", "");
      }
    }
  }
}


// ================== SETUP ==================
void setup() {

  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi da ket noi");
  Serial.println(WiFi.localIP());

  client.setInsecure();  // bắt buộc cho Telegram
}


// ================== LOOP ==================
void loop() {

  if (millis() - lastTimeBotRan > bot_delay) {

    Serial.println("Dang check tin nhan...");

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    }

    lastTimeBotRan = millis();
  }
}