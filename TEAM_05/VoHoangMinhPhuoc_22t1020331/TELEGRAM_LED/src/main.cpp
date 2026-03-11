#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WIFI
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// TELEGRAM
#define BOTtoken "8633664176:AAGcwbrw_CFlhMj3AchCXqA6km1VAIQS5oM"
#define GROUP_ID "-5158052780"

// PIN
const int motionSensor = 27;
const int ledPin = 23;

bool motionDetected = false;

unsigned long lastTimeBotRan;
int delayBetweenChecks = 1000;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//////////////////////////////////////////////////////

void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

//////////////////////////////////////////////////////

void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.print("Nhan duoc: ");
    Serial.println(text);

    if (chat_id != GROUP_ID) continue;

    // START
    if (text.indexOf("/start") >= 0) {

      String welcome = "Xin chao " + from_name + "\n";
      welcome += "Danh sach lenh:\n";
      welcome += "/led_on : Bat den\n";
      welcome += "/led_off : Tat den\n";
      welcome += "/get_state : Trang thai den\n";

      bot.sendMessage(GROUP_ID, welcome, "");
    }

    // LED ON
    if (text.indexOf("/led_on") >= 0) {

      digitalWrite(ledPin, HIGH);
      bot.sendMessage(GROUP_ID, "LED da BAT", "");
    }

    // LED OFF
    if (text.indexOf("/led_off") >= 0) {

      digitalWrite(ledPin, LOW);
      bot.sendMessage(GROUP_ID, "LED da TAT", "");
    }

    // GET STATE
    if (text.indexOf("/get_state") >= 0) {

      String state = digitalRead(ledPin) ? "ON" : "OFF";
      bot.sendMessage(GROUP_ID, "Trang thai LED: " + state, "");
    }
  }
}

//////////////////////////////////////////////////////

void setup() {

  Serial.begin(115200);

  pinMode(motionSensor, INPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.print("Connecting Wifi: ");

  WiFi.mode(WIFI_STA);

  // DNS fix
  IPAddress dns(8,8,8,8);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns);

  WiFi.begin(ssid, password);

  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");
    delay(500);

  }

  Serial.println();
  Serial.println("WiFi connected");

  if (bot.sendMessage(GROUP_ID, "ESP32 IoT da san sang!", "")) {

    Serial.println("Telegram ket noi thanh cong");

  } else {

    Serial.println("Loi ket noi Telegram");

  }
}

//////////////////////////////////////////////////////

void loop() {

  static uint count_ = 0;

  // PIR SENSOR
  if (motionDetected) {

    count_++;

    Serial.println("Motion detected");

    String msg = "Phat hien chuyen dong: " + String(count_);

    bot.sendMessage(GROUP_ID, msg, "");

    motionDetected = false;
  }

  // TELEGRAM
  if (millis() > lastTimeBotRan + delayBetweenChecks) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}
