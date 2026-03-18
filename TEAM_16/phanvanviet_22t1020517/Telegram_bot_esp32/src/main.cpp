#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 1. CẤU HÌNH WIFI (Mặc định cho Wokwi)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// 2. CẤU HÌNH TELEGRAM BOT
// Thay "YOUR_BOT_TOKEN" bằng Token lấy từ BotFather trên Telegram
#define BOT_TOKEN "8765576105:AAH26Mc4HSN7zT_PfSVgxZBENKqrX-pW5FE"

// Cấu hình Group ID (Đã điền Group ID từ ảnh của bạn)
#define GROUP_CHAT_ID "-5242520374"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// Thời gian trễ giữa các lần kiểm tra tin nhắn mới (mili giây)
const unsigned long BOT_MTBS = 1000;
unsigned long bot_lasttime = 0;

// 3. CẤU HÌNH CHÂN KẾT NỐI (Dựa theo diagram.json)
const int pirPin = 27; // Chân OUT của PIR
const int ledPin = 23; // Chân điều khiển LED màu xanh

bool ledState = LOW;
int lastPirState = LOW; // Thêm biến lưu trạng thái PIR cũ để gửi cảnh báo 1 lần

// Hàm xử lý khi có tin nhắn mới tới từ Telegram
void handleNewMessages(int numNewMessages) {
  Serial.print("Có tin nhắn mới: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (from_name == "") from_name = "Guest";

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }
    else if (text == "/led_off") {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED is OFF", "");
    }
    else if (text == "/get_state") {
      String stateMessage = "";
      
      // Lấy trạng thái LED
      if (ledState == HIGH) {
        stateMessage += "LED is ON\n";
      } else {
        stateMessage += "LED is OFF\n";
      }

      // Lấy trạng thái cảm biến PIR
      int pirState = digitalRead(pirPin);
      if (pirState == HIGH) {
        stateMessage += "Cảm biến PIR: Phát hiện chuyển động!";
      } else {
        stateMessage += "Cảm biến PIR: Không có chuyển động.";
      }

      bot.sendMessage(chat_id, stateMessage, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Cấu hình chân IO
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(ledPin, ledState);

  // Kết nối WiFi
  Serial.print("Đang kết nối tới mạng WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Thiết lập chứng chỉ bảo mật cho Telegram

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nĐã kết nối WiFi.");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());

  // Cấu hình thời gian (Yêu cầu bắt buộc để Telegram API hoạt động qua HTTPS)
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println("\nĐã cập nhật thời gian.");
}

void loop() {
  // Liên tục kiểm tra tin nhắn Telegram mỗi giây
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("Đang xử lý tin nhắn...");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  // Kiểm tra cảm biến PIR và tự động gửi cảnh báo vào Group
  int currentPirState = digitalRead(pirPin);
  if (currentPirState == HIGH && lastPirState == LOW) {
    Serial.println("Phát hiện chuyển động! Gửi tin nhắn vào Group...");
    bot.sendMessage(GROUP_CHAT_ID, "⚠️ CẢNH BÁO: Phát hiện có chuyển động!", "");
    lastPirState = HIGH;
  } else if (currentPirState == LOW && lastPirState == HIGH) {
    lastPirState = LOW; // Reset trạng thái khi hết chuyển động
  }
}