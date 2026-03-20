#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define BOTtoken "8318140658:AAGSL58aWINuKzOClbhQV-pvm1KE0g1mRGM"
#define CHAT_ID "-5214203269"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// check message delay
int bot_delay = 1000;
unsigned long lastTimeBotRan;

// LED
const int ledPin = 23;
bool ledState = LOW;

// PIR
const int motionSensor = 27;
bool motionDetected = false;

// interrupt PIR
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// format string
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