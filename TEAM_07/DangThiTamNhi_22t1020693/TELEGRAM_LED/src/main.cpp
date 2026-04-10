<<<<<<< HEAD
#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
=======
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 1. Cấu hình WiFi cho Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// 2. Cấu hình Telegram (THAY THÔNG TIN CỦA BẠN VÀO ĐÂY)
#define BOTtoken "token_id"  // Copy đủ cả số và chữ từ BotFather
#define GROUP_ID "-5179083739"   
// 3. Khai báo chân Pin theo sơ đồ Wokwi
const int ledPin = 23;       
const int motionSensor = 27; 

bool motionDetected = false;
unsigned long lastTimeBotRan;
const unsigned long botRequestDelay = 1000; // 1 giây quét tin nhắn 1 lần

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Hàm ngắt: Chạy ngay lập tức khi PIR phát hiện chuyển động
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// Hàm xử lý lệnh nhận được từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    
    Serial.println("Nhận lệnh: " + text);

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, " Đèn LED đã BẬT", "");
    } 
    else if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, " Đèn LED đã TẮT", "");
    } 
    else if (text == "/get_state") {
      String status = (digitalRead(ledPin) == HIGH) ? "đang SÁNG" : "đang TẮT";
      bot.sendMessage(chat_id, " Trạng thái: Đèn " + status, "");
    } 
    else if (text == "/start") {
      bot.sendMessage(chat_id, "Chào Tâm Nhi! Hệ thống sẵn sàng. Hãy dùng Menu lệnh.", "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Cấu hình chân Out/In
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(motionSensor, INPUT_PULLUP);
  
  // Thiết lập ngắt cho chân 27
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Kết nối WiFi
  Serial.print("Dang ket noi WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println("\nWiFi Connected!");
  
  // Thông báo khi khởi động xong
  if(bot.sendMessage(GROUP_ID, " Hệ thống đã Online!", "")) {
    Serial.println("Da gui thong bao khoi dong len Telegram");
  } else {
    Serial.println("LOI: Khong the gui tin nhan khoi dong. Kiem tra GROUP_ID!");
  }
}

void loop() {
  // PHẦN 1: Xử lý Cảm biến PIR
  if(motionDetected){
    Serial.println("PIR: Phat hien chuyen dong!");
    
    // Gửi cảnh báo
    if(bot.sendMessage(GROUP_ID, "CẢNH BÁO: Có chuyển động!", "")) {
      Serial.println("Telegram: Da gui canh bao thanh cong");
    } else {
      Serial.println("Telegram: Gui canh bao that bai");
    }
    
    // Chống nhiễu và đợi cảm biến ổn định lại
    delay(2000); 
    motionDetected = false; 
  }

  // PHẦN 2: Quét tin nhắn mới từ người dùng
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    if (numNewMessages > 0) {
      Serial.print("Co "); Serial.print(numNewMessages); Serial.println(" tin nhan moi.");
      handleNewMessages(numNewMessages);
    }
    
    lastTimeBotRan = millis();
  }
>>>>>>> 5602a92cc9d530667b86fb8a7dfbb50a7b85c4b2
}