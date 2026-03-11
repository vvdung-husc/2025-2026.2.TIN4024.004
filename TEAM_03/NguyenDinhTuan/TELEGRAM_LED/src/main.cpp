#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define BOTtoken "8762169458:AAHQGTWKy0zZmKkx334JhZw1FBxG5kXm6p8"
#define GROUP_ID "-1003509405398"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27;
const int ledPin = 23;
bool motionDetected = false;
bool ledState = false;

int Bot_mtbs = 200;
long Bot_lasttime;

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

void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    // Ignore messages sent by the bot itself
    if (bot.messages[i].from_id == bot.messages[i].chat_id) continue;
    if (bot.messages[i].type != "message") continue;

    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/start") {
      String welcome =
        "Bot đã khởi động.\n"
        "Sử dụng các lệnh sau để điều khiển đèn LED.\n"
        "Gửi /led_on để bật đèn LED\n"
        "Gửi /led_off để tắt đèn LED\n"
        "Gửi /get_state để yêu cầu trạng thái LED hiện tại";
      bot.sendMessage(chat_id, welcome);
    }
    else if (text == "/led_on") {
      ledState = true;
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "Đèn LED đã được BẬT.");
    }
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "Đèn LED đã được TẮT.");
    }
    else if (text == "/get_state") {
      String state = ledState ? "Đèn LED hiện đang: BẬT" : "Đèn LED hiện đang: TẮT";
      bot.sendMessage(chat_id, state);
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

  Serial.println("\nWiFi connected");

  bot.sendMessage(GROUP_ID,
    "IoT Developer đã kết nối - CNTT K46\nGửi /start để xem hướng dẫn");
}

void loop() {
  static uint count_ = 0;

  if (motionDetected) {
    ++count_;
    Serial.print(count_);
    Serial.println(". MOTION DETECTED => Sending to Telegram");
    String msg = StringFormat("%u => Motion detected!", count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    motionDetected = false;
  }

  if (millis() > Bot_lasttime + Bot_mtbs) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    Bot_lasttime = millis();
  }
}