#define BLYNK_TEMPLATE_ID "TMPL6gNKFdUl0"
#define BLYNK_TEMPLATE_NAME "HeartRate"
#define BLYNK_AUTH_TOKEN "-NPNRXsFKFo4lJelz8Yje-0ofnDwipMt"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Telegram Bot
#define BOT_TOKEN "8713058710:AAGXpC0DMvG0jOtgkUIOIZfIH923QjD5tRI"
#define CHAT_ID "-5136107665"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

#define SENSOR_PIN 34
#define LED_PIN 2

int bpm = 0;
int timeCounter = 0; // CSV

// LED nhấp nháy không delay
unsigned long previousMillisLED = 0;
int ledBlinkCount = 0;
bool ledState = false;

// Telegram streaming
bool streaming = false;
unsigned long previousMillisTelegram = 0;

// Telegram tự động cảnh báo
bool alertSentHigh = false;
bool alertSentLow = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
  client.setInsecure();

  Serial.println("Time,BPM"); // CSV header
  Serial.println("System Started...");
}

void loop() {
  Blynk.run();

  // Đọc nhịp tim
  int value = analogRead(SENSOR_PIN);
  bpm = map(value, 0, 4095, 60, 120);

  // CSV Serial
  Serial.print(timeCounter);
  Serial.print(",");
  Serial.println(bpm);
  timeCounter++;

  // Gửi Blynk
  Blynk.virtualWrite(V0, bpm);

  unsigned long currentMillis = millis();

  // LED nhấp nháy khi BPM > 100 (5 lần, 100ms sáng/tắt)
  if (bpm > 100) {
    if (currentMillis - previousMillisLED >= 100) {
      previousMillisLED = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      if (!ledState) ledBlinkCount++; // tính khi LED tắt
      if (ledBlinkCount >= 5) ledBlinkCount = 0; // reset sau 5 lần
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    ledBlinkCount = 0;
  }

  // Telegram nhận lệnh
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;
      String chat_id = String(bot.messages[i].chat_id);

      if (text == "/bpm") {
        String status = (bpm < 60) ? "Thấp" : (bpm <= 100) ? "Bình thường" : "Cao";
        String msg = "Nhịp tim hiện tại: " + String(bpm) + " BPM\nTrạng thái: " + status;
        bot.sendMessage(chat_id, msg, "");
      } 
      else if (text == "/start") {
        streaming = true;
        bot.sendMessage(chat_id, "✅ Bắt đầu gửi nhịp tim liên tục...", "");
      } 
      else if (text == "/stop") {
        streaming = false;
        bot.sendMessage(chat_id, "🛑 Dừng gửi nhịp tim liên tục.", "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  // Gửi nhịp tim liên tục khi streaming (1 giây/lần)
  if (streaming && currentMillis - previousMillisTelegram >= 1000) {
    previousMillisTelegram = currentMillis;
    String msg = "Nhịp tim: " + String(bpm) + " BPM";
    bot.sendMessage(CHAT_ID, msg, "");
  }

  // Telegram tự động cảnh báo khi nhịp tim bất thường
  if (bpm > 100) {
    if (!alertSentHigh) {
      bot.sendMessage(CHAT_ID, "⚠️ Nhịp tim cao: " + String(bpm) + " BPM", "");
      alertSentHigh = true;
      alertSentLow = false; // reset cảnh báo thấp
    }
  } 
  else if (bpm < 60) {
    if (!alertSentLow) {
      bot.sendMessage(CHAT_ID, "⚠️ Nhịp tim thấp: " + String(bpm) + " BPM", "");
      alertSentLow = true;
      alertSentHigh = false; // reset cảnh báo cao
    }
  } 
  else {
    // Bình thường → reset cảnh báo
    alertSentHigh = false;
    alertSentLow = false;
  }
}