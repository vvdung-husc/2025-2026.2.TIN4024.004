
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
// Token của bot Telegram
#define BOTtoken "7891564074:AAG2GAwL7bQwSJF4e6IeAoq8Jh0UfSdH2zA"
#define CHAT_ID "-5276017972" //chat id của nhóm

WiFiClientSecure client; // Khởi tạo client HTTPS cho Telegram Bot
UniversalTelegramBot bot(BOTtoken, client); // Khởi tạo bot Telegram với token và client HTTPS

int bot_delay = 1000; // thời gian kiểm tra tin nhắn mới (1s)
unsigned long lastTimeBotRan; // biến lưu thời điểm lần cuối bot kiểm tra tin nhắn mới

// LED
const int ledPin = 23;
bool ledState = LOW;

// 
const int motionSensor = 27;
bool motionDetected = false; // biến lưu trạng thái phát hiện chuyển động

// hàm xử lý khi phát hiện chuyển động
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

String StringFormat(const char* fmt, ...) {
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

  return String(s);
}


// xử lý tin nhắn telegram
void handleNewMessages(int numNewMessages) {

  Serial.println("Handling New Message");

  for (int i = 0; i < numNewMessages; i++) {
// lấy chat id của người gửi tin nhắn
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String user_text = bot.messages[i].text;
    String your_name = bot.messages[i].from_name;

    Serial.println(user_text);

    if (user_text == "/start") {

      String welcome = "Welcome " + your_name + "\n";
      welcome += "Commands:\n";
      welcome += "/led_on : turn LED ON\n";
      welcome += "/led_off : turn LED OFF\n";
      welcome += "/get_state : LED state\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (user_text == "/led_on") {

      ledState = HIGH;
      digitalWrite(ledPin, ledState);

      bot.sendMessage(chat_id, "LED ON", "");
    }

    if (user_text == "/led_off") {

      ledState = LOW;
      digitalWrite(ledPin, ledState);

      bot.sendMessage(chat_id, "LED OFF", "");
    }

    if (user_text == "/get_state") {

      if (digitalRead(ledPin)) {
        bot.sendMessage(chat_id, "LED is ON", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}



void setup() {

  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  pinMode(motionSensor, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // connect wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi connected");

  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "ESP32 System Started", "");
}



void loop() {

  // PIR detect
  static uint count_ = 0;

  if (motionDetected) {

    count_++;

    Serial.println("Motion detected");

    String msg = StringFormat("%u : Motion detected!", count_);

    bot.sendMessage(CHAT_ID, msg.c_str());

    motionDetected = false;
  }


  // telegram message check
  if (millis() > lastTimeBotRan + bot_delay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      Serial.println("Got Response!");

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}
