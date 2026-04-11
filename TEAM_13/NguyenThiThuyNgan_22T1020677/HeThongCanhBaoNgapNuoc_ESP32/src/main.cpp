#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h> // Đảm bảo đã cài thư viện này

const char* ssid = "Wokwi-GUEST";
const char* password = "";
#define BOTtoken "8561789085:AAFqrc1k9xGSnNbLpnx57g4n8aEYq7dMBqc"
#define CHAT_ID "8726144638"

const int POT_PIN = 34;
const int LED_PIN = 18;
const int BUZZER_PIN = 26;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.print("Dang ket noi WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi da ket noi!");

  client.setInsecure();
  digitalWrite(LED_PIN, LOW);
}

void playAlarm() {
  for(int i=0; i<3; i++) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000); 
    delay(200);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    delay(200);
  }
}

void loop() {
  // Đọc giá trị Analog (0 - 4095)
  int val = analogRead(POT_PIN);
  // Chuyển đổi sang phần trăm (0 - 100%)
  int percent = map(val, 0, 4095, 0, 100);

  // 1. Hiển thị giá trị ra Serial Monitor hàng giây
  Serial.print("Gia tri do am/muc nuoc hien tai: ");
  Serial.print(percent);
  Serial.println("%");

  if (percent > 70) {
    Serial.println("⚠️ CANH BAO NGAP!");
    
    // 2. Gui tin nhan kem gia tri cu the qua Telegram
    String message = "⚠️ Nguy hiểm! Phát hiện ngập lụt!\n";
    message += "Mức nước đo được: " + String(percent) + "%";
    
    bot.sendMessage(CHAT_ID, message, "");
    
    playAlarm();
    delay(5000); // Tránh gửi tin nhắn quá dồn dập
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }
  
  delay(1000); // Đọc lại sau mỗi giây
}