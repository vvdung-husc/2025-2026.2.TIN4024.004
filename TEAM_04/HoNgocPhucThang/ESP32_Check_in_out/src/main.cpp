#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// --- CẤU HÌNH ---
const char* ssid = "Wokwi-GUEST";
const char* password = ""; 
const char* botToken = "8161835889:AAEfRs92rg80jkTCOpDUR7VdEpOIRNs9spQ";
const char* chat_id = "-5275861628"; 

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

const int sensorA = 14; 
const int sensorB = 27;

int count = 0;
int state = 0; 
unsigned long timer = 0;

void setup() {
  Serial.begin(115200);
  pinMode(sensorA, INPUT_PULLUP);
  pinMode(sensorB, INPUT_PULLUP);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  client.setInsecure(); 
  bot.sendMessage(chat_id, "✅ *Hệ thống đã sẵn sàng!*", "Markdown");
}

void loop() {
  bool a = (digitalRead(sensorA) == LOW); // Nút nhấn trả về LOW
  bool b = (digitalRead(sensorB) == LOW);

  if (state == 0) {
    if (a && !b) { state = 1; timer = millis(); Serial.println("Đã qua A..."); }
    else if (b && !a) { state = 2; timer = millis(); Serial.println("Đã qua B..."); }
  } 
  else if (state == 1) { // Đang chờ B để xác nhận VÀO
    if (b) {
      count++;
      bot.sendMessage(chat_id, "🚶 *VÀO:* Một người mới.\n📊 Tổng: " + String(count), "Markdown");
      state = 3; 
    }
  } 
  else if (state == 2) { // Đang chờ A để xác nhận RA
    if (a) {
      if (count > 0) count--;
      bot.sendMessage(chat_id, "🚶 *RA:* Một người rời đi.\n📊 Tổng: " + String(count), "Markdown");
      state = 3;
    }
  }

  // Quá 3 giây không bấm nút tiếp theo thì hủy
  if ((state == 1 || state == 2) && (millis() - timer > 3000)) {
    state = 0;
    Serial.println("Hủy lượt đếm (Quá thời gian)");
  }

  // Chờ nhả cả 2 nút mới cho đếm tiếp
  if (state == 3 && !a && !b) {
    delay(200);
    state = 0;
  }
}