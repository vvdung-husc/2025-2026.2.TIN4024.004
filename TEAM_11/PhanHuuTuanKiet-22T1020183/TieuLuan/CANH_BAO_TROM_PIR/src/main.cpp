#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ESP_Mail_Client.h>

// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= TELEGRAM =================
#define BOTtoken "8732291282:AAHTmlRrBBeNN-R6b7jJ4g5atqrQjxjt1P4"
#define GROUP_ID "-5182604324"

// ================= EMAIL =================
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "22T1020183@husc.edu.vn"
#define AUTHOR_PASSWORD "afdw lrqt pixz enzn"
#define RECIPIENT_EMAIL "22T1020183@husc.edu.vn"

// ================= OBJECT =================
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
SMTPSession smtp;

// ================= PIN =================
const int motionSensor = 27;
const int ledPin = 22;
const int buzzerPin = 18;

// ================= BIẾN =================
volatile bool motionDetected = false;
int alertCount = 0;
unsigned long lastAlert = 0;
const long cooldown = 10000; // chống spam 10s

// ================= NGẮT =================
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// ================= GỬI EMAIL =================
void sendEmail(String msg) {
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  SMTP_Message message;
  message.sender.name = "ESP32 Security";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = " CẢNH BÁO TRỘM!";
  message.addRecipient("User", RECIPIENT_EMAIL);
  message.text.content = msg.c_str();
  message.text.charSet = "utf-8";

  Serial.print("Đang gửi Email... ");

  if (!smtp.connect(&session)) {
    Serial.println("❌ SMTP lỗi!");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("❌ Gửi Email thất bại: " + smtp.errorReason());
  } else {
    Serial.println("✅ Email đã gửi!");
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(motionSensor, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // --- WIFI ---
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println("\nWiFi connected");
  Serial.println("HE THONG CHONG TROM SAN SANG");

  // báo khởi động
  digitalWrite(ledPin, HIGH);
  digitalWrite(buzzerPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  bot.sendMessage(GROUP_ID, "Hệ thống chống trộm đã khởi động!");
}

// ================= LOOP =================
void loop() {

  if (motionDetected && millis() - lastAlert > cooldown) {
    lastAlert = millis();
    alertCount++;

    Serial.println("Phát hiện chuyển động!");
    Serial.printf("%d. MOTION DETECTED => Waiting to send to Telegram", alertCount);

    // bật LED
    digitalWrite(ledPin, HIGH);

    // buzzer cảnh báo
    for (int i = 0; i < 5; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(200);
      digitalWrite(buzzerPin, LOW);
      delay(200);
    }

    String msg = "CẢNH BÁO TRỘM: Phát hiện chuyển động bất thường!!!";

    // --- TELEGRAM ---
    if (bot.sendMessage(GROUP_ID, msg, "")) {
      Serial.printf("%d. Sent successfully to Telegram: Motion Detected", alertCount);
    } else {
      Serial.println("❌ Loi gui Telegram!");
    }

    // --- EMAIL ---
    sendEmail(msg);

    delay(3000);
    digitalWrite(ledPin, LOW);

    motionDetected = false;
  }
}