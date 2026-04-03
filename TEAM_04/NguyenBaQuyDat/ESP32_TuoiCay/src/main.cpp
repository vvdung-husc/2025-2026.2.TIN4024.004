#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "DHT.h"

// --- Cấu hình WiFi cho Wokwi ---
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// --- Cấu hình Telegram ---
#define BOT_TOKEN "8434631302:AAEAY6RYBuwSSWisvWrYtEOt56n3TWteuiA"
#define CHAT_ID "-5014770778"

// --- Cấu hình phần cứng ---
#define PIN_DHT 12
#define PIN_LED 23
#define PIN_PUMP 18 // Chân giả lập máy bơm
#define DHTTYPE DHT22

DHT dht(PIN_DHT, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastTimeBotRan;
const unsigned long botRequestDelay = 1000; // 1 giây quét lệnh 1 lần
float temperature, humidity;
bool isAlerted = false; // Biến cờ để tránh gửi tin nhắn cảnh báo liên tục

void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
      continue;

    String text = bot.messages[i].text;

    if (text == "/on")
    {
      digitalWrite(PIN_LED, HIGH);
      bot.sendMessage(CHAT_ID, "Máy bớm đã được BẬT", "");
    }
    else if (text == "/off")
    {
      digitalWrite(PIN_LED, LOW);
      bot.sendMessage(CHAT_ID, "Máy bơm đã được TẮT", "");
    }
    else if (text == "/get_weather")
    {
      String msg = "🌡 Nhiệt độ: " + String(temperature, 1) + "°C\n";
      msg += "💧 Độ ẩm: " + String(humidity, 1) + "%";
      bot.sendMessage(CHAT_ID, msg, "");
    }
    else if (text == "/get_state")
    {
      String state = "🤖 TRẠNG THÁI HỆ THỐNG:\n";
      state += "Đèn: " + String(digitalRead(PIN_LED) ? "BẬT" : "TẮT") + "\n";
      state += "Máy bơm: " + String(digitalRead(PIN_PUMP) ? "ĐANG TƯỚI" : "NGỪNG") + "\n";
      state += "Nhiệt độ hiện tại: " + String(temperature, 1) + "°C";
      bot.sendMessage(CHAT_ID, state, "");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_PUMP, LOW);

  dht.begin();

  // Kết nối WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Bỏ qua kiểm tra SSL để tăng tốc độ cho ESP32
  client.setInsecure();

  bot.sendMessage(CHAT_ID, "🚀 Hệ thống ESP32 Tưới cây tự động đã sẵn sàng!", "");
}

void loop()
{
  // Đọc cảm biến DHT22
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h))
  {
    temperature = t;
    humidity = h;
  }

  // Logic tự động: Nhiệt độ > 38°C
  if (temperature > 38.0)
  {
    digitalWrite(PIN_PUMP, HIGH); // Tự động tưới
    digitalWrite(PIN_LED, HIGH);  // Bật đèn sáng

    if (!isAlerted)
    { // Chỉ gửi cảnh báo 1 lần khi vượt ngưỡng
      String alertMsg = "⚠️ CẢNH BÁO NGUY HIỂM!\n";
      alertMsg += "Nhiệt độ đã vượt ngưỡng: " + String(temperature, 1) + "°C\n";
      alertMsg += "Hệ thống đang tự động tưới cây và bật đèn!";
      bot.sendMessage(CHAT_ID, alertMsg, "");
      isAlerted = true;
    }
  }
  else
  {
    // Nếu nhiệt độ giảm xuống dưới ngưỡng an toàn (ví dụ 37°C để tránh dao động)
    if (temperature < 37.0 && isAlerted)
    {
      digitalWrite(PIN_PUMP, LOW);
      bot.sendMessage(CHAT_ID, "✅ Nhiệt độ đã ổn định. Ngừng tưới tự động.", "");
      isAlerted = false;
    }
  }

  // Kiểm tra tin nhắn Telegram
  if (millis() > lastTimeBotRan + botRequestDelay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}