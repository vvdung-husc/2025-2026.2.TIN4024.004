#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- 1. THÔNG TIN WIFI CỦA WOKWI ---

const char* ssid = "Wokwi-GUEST"; 
const char* password = "";        

// --- 2. THÔNG TIN TELEGRAM ---
// Sử dụng Token CHUẨN mà bạn đã lấy thành công
#define BOTtoken "8520043856:AAFIzLAraVLL7NLaGT31nN23J5lzbkhxzbo"  

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// --- 3. CẤU HÌNH PHẦN CỨNG  ---

const int ledPin = 23; 
bool ledState = false; // Trạng thái LED ban đầu là tắt

// Cài đặt thời gian kiểm tra tin nhắn (1 giây)
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// --- 4. HÀM XỬ LÝ LỆNH TỪ TELEGRAM  ---

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    // Lấy ID của nơi gửi để biết đường nhắn lại
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    // Phản hồi các lệnh (Ai nhắn cũng phục vụ hết)
    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      digitalWrite(ledPin, HIGH); 
      ledState = true;            
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }
    else if (text == "/led_off") {
      digitalWrite(ledPin, LOW);  
      ledState = false;           
      bot.sendMessage(chat_id, "LED is OFF", ""); // Hoặc "LED đã tắt"
    }
    else if (text == "/get_state") {
      if (ledState) {
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
  digitalWrite(ledPin, LOW); // Tắt LED lúc mới khởi động

  // Kết nối WiFi
  Serial.print("Đang kết nối WiFi");
  WiFi.begin(ssid, password);

  // --- THAY ĐỔI CỰC KỲ QUAN TRỌNG CHO WOKWI SIMULATOR ---
  // Bỏ qua kiểm tra chứng chỉ SSL để tránh lỗi sai lệch thời gian thực
  client.setInsecure(); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối WiFi thành công!");
}

void loop() {
  // Liên tục kiểm tra tin nhắn từ Telegram
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    while (numNewMessages) {
      Serial.println("Nhận được lệnh từ Telegram!");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}