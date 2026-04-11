#include <Arduino.h> // Bắt buộc phải có khi dùng PlatformIO
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";  

// --- THÔNG TIN TELEGRAM BOT ---
#define BOT_TOKEN "8777360752:AAGuvBPL-5pvq1FPM7ZzQAxNWWGNJcDGEP4"
#define CHAT_ID "8796555849"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// --- KHAI BÁO CÁC CHÂN KẾT NỐI TRÊN MẠCH THẬT ---
const int gasPin = 34;    // Chân tín hiệu (A0/SIG) của cảm biến MQ-6
const int buzzerPin = 2;  // Chân còi báo động (Buzzer)
const int gasThreshold = 2000; // Ngưỡng rò rỉ khí gas (Có thể tinh chỉnh sau)

unsigned long lastTimeBotRan = 0;
const unsigned long botRequestDelay = 10000; // 10 giây gửi tối đa 1 tin

void setup() {
  Serial.begin(115200);

  pinMode(gasPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW); 

  Serial.print("Đang kết nối WiFi...");
  WiFi.begin(ssid, password);
  client.setInsecure(); 

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nĐã kết nối WiFi thành công!");

  // Gửi tin nhắn kiểm tra khi mạch ESP32 thật vừa bật lên
  bot.sendMessage(CHAT_ID, "🟢 [MẠCH THẬT] Hệ thống báo rò rỉ khí gas đã kết nối thành công!", "");
}

void loop() {
  int gasLevel = analogRead(gasPin);
  Serial.print("Nồng độ gas: ");
  Serial.println(gasLevel);

  if (gasLevel > gasThreshold) {
    digitalWrite(buzzerPin, HIGH);

    if (millis() - lastTimeBotRan > botRequestDelay) {
      Serial.println("⚠ Phát hiện rò rỉ! Đang gửi cảnh báo qua Telegram...");
      String message = "⚠ CẢNH BÁO: Phát hiện rò rỉ khí gas!\nNồng độ hiện tại: " + String(gasLevel);
      bot.sendMessage(CHAT_ID, message, "");
      lastTimeBotRan = millis(); 
    }
  } else {
    digitalWrite(buzzerPin, LOW);
  }
  delay(1000); 
}