#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===================== CẤU HÌNH =====================
#define SS_PIN    5
#define RST_PIN   4
#define SERVO_PIN 13
#define LED_GREEN 2
#define LED_RED   15
#define BUZZER    25

const char* ssid     = "Wokwi-GUEST";
const char* password = "";

const String BOT_TOKEN = "8681635574:AAG6Zom7IGZwayo0r21Alo9fl_OduD7SoQ8";
const String CHAT_ID   = "-5180535191";
const String API_BASE  = "https://api.telegram.org/bot" + BOT_TOKEN;

// ===================== KHỞI TẠO =====================
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo lockServo;

String authorizedUIDs[10] = {
  "A1B2C3D4", "11223344",
  "", "", "", "", "", "", "", ""
};
int numAuthorized = 2;

bool isDoorOpen = false;
unsigned long doorOpenTime = 0;
const unsigned long DOOR_TIMEOUT = 5000;

long lastUpdateId = 0;
unsigned long lastPollTime = 0;
const unsigned long POLL_INTERVAL = 2000;

// ===================== TELEGRAM GỬI CÓ INLINE KEYBOARD =====================

// Gửi tin nhắn thường (không có button)
void sendTelegram(String message) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(API_BASE + "/sendMessage");
  http.addHeader("Content-Type", "application/json");
  message.replace("\"", "'");
  String payload = "{\"chat_id\":\"" + CHAT_ID + "\","
                   "\"text\":\"" + message + "\","
                   "\"parse_mode\":\"Markdown\"}";
  http.POST(payload);
  http.end();
}

// Gửi tin nhắn kèm Inline Keyboard buttons
void sendMenuMain() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(API_BASE + "/sendMessage");
  http.addHeader("Content-Type", "application/json");

  // Layout: 2 nút / hàng
  String keyboard =
    "{"
      "\"inline_keyboard\":["
        "[{\"text\":\"🔓 Mở Cửa\",\"callback_data\":\"/open\"},"
         "{\"text\":\"🔒 Đóng Cửa\",\"callback_data\":\"/close\"}],"
        "[{\"text\":\"📊 Trạng Thái\",\"callback_data\":\"/status\"},"
         "{\"text\":\"📋 Danh Sách Thẻ\",\"callback_data\":\"/listcards\"}],"
        "[{\"text\":\"➕ Thêm Thẻ Mẫu\",\"callback_data\":\"/addcard_demo\"},"
         "{\"text\":\"➖ Xóa Thẻ Mẫu\",\"callback_data\":\"/removecard_demo\"}],"
        "[{\"text\":\"❓ Hướng Dẫn\",\"callback_data\":\"/help\"}]"
      "]"
    "}";

  String payload = "{\"chat_id\":\"" + CHAT_ID + "\","
                   "\"text\":\"🏠 *BẢNG ĐIỀU KHIỂN KHÓA CỬA THÔNG MINH*\\nChọn chức năng bên dưới:\","
                   "\"parse_mode\":\"Markdown\","
                   "\"reply_markup\":" + keyboard + "}";
  http.POST(payload);
  http.end();
}

// Trả lời callback query (bắt buộc để tắt loading trên button)
void answerCallbackQuery(String callbackQueryId, String text) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(API_BASE + "/answerCallbackQuery");
  http.addHeader("Content-Type", "application/json");
  text.replace("\"", "'");
  String payload = "{\"callback_query_id\":\"" + callbackQueryId + "\","
                   "\"text\":\"" + text + "\","
                   "\"show_alert\":false}";
  http.POST(payload);
  http.end();
}

// ===================== TIỆN ÍCH =====================

String getCardUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

bool isAuthorized(String uid) {
  for (int i = 0; i < 10; i++)
    if (authorizedUIDs[i] == uid) return true;
  return false;
}

bool addCard(String uid) {
  if (isAuthorized(uid)) return false;
  for (int i = 0; i < 10; i++) {
    if (authorizedUIDs[i] == "") {
      authorizedUIDs[i] = uid;
      numAuthorized++;
      return true;
    }
  }
  return false;
}

bool removeCard(String uid) {
  for (int i = 0; i < 10; i++) {
    if (authorizedUIDs[i] == uid) {
      authorizedUIDs[i] = "";
      numAuthorized--;
      return true;
    }
  }
  return false;
}

// ===================== ĐIỀU KHIỂN CỬA =====================

void openDoor(String source) {
  isDoorOpen   = true;
  doorOpenTime = millis();
  lockServo.write(90);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED,   LOW);
  digitalWrite(BUZZER, HIGH); delay(100); digitalWrite(BUZZER, LOW);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("  CUA DA MO!    ");
  lcd.setCursor(0, 1); lcd.print(source.substring(0, 16));
  Serial.println(">> Mo cua: " + source);
}

void closeDoor() {
  isDoorOpen = false;
  lockServo.write(0);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED,   HIGH);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(" HE THONG KHOA  ");
  lcd.setCursor(0, 1); lcd.print(" Quet the de mo ");
  sendTelegram("🔒 Cửa đã đóng.");
  Serial.println(">> Dong cua");
}

void denyAccess(String uid) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_RED, HIGH); delay(150);
    digitalWrite(LED_RED, LOW);  delay(150);
  }
  digitalWrite(LED_RED, HIGH);
  digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(" TRUY CAP BI TU ");
  lcd.setCursor(0, 1); lcd.print(uid.substring(0, 16));
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(" HE THONG KHOA  ");
  lcd.setCursor(0, 1); lcd.print(" Quet the de mo ");
  sendTelegram("⚠️ *CẢNH BÁO!*\nThẻ không hợp lệ!\nUID: " + uid);
}

// ===================== XỬ LÝ LỆNH (dùng chung cho text và callback) =====================

void handleCommand(String cmd, String sender, String callbackQueryId = "") {
  cmd.trim();

  // Trả lời callback để tắt loading button
  if (callbackQueryId != "") {
    answerCallbackQuery(callbackQueryId, "Đang xử lý...");
  }

  if (cmd == "/open") {
    if (!isDoorOpen) {
      openDoor("Telegram");
      sendTelegram("✅ *" + sender + "* đã mở cửa từ xa!\nCửa sẽ tự đóng sau 5 giây.");
    } else {
      sendTelegram("ℹ️ Cửa đang mở rồi!");
    }
  }

  else if (cmd == "/close") {
    if (isDoorOpen) {
      closeDoor();
      sendTelegram("🔒 *" + sender + "* đã đóng cửa từ xa!");
    } else {
      sendTelegram("ℹ️ Cửa đang đóng rồi!");
    }
  }

  else if (cmd == "/status") {
    String st = isDoorOpen ? "🔓 ĐANG MỞ" : "🔒 ĐANG ĐÓNG";
    String wifi = (WiFi.status() == WL_CONNECTED) ? "✅ Kết nối" : "❌ Mất kết nối";
    String msg = "📊 *TRẠNG THÁI HỆ THỐNG*\n";
    msg += "Cửa: " + st + "\n";
    msg += "WiFi: " + wifi + "\n";
    msg += "IP: " + WiFi.localIP().toString() + "\n";
    msg += "Số thẻ đăng ký: " + String(numAuthorized) + "/10";
    sendTelegram(msg);
  }

  else if (cmd == "/listcards") {
    String msg = "📋 *DANH SÁCH THẺ HỢP LỆ*\n";
    int count = 0;
    for (int i = 0; i < 10; i++) {
      if (authorizedUIDs[i] != "") {
        count++;
        msg += String(count) + ". `" + authorizedUIDs[i] + "`\n";
      }
    }
    if (count == 0) msg += "_(Chưa có thẻ nào)_";
    sendTelegram(msg);
  }

  // Button demo: thêm thẻ mẫu AABBCCDD
  else if (cmd == "/addcard_demo") {
    String uid = "AABBCCDD";
    if (addCard(uid)) {
      sendTelegram("✅ *Thêm thẻ mẫu thành công!*\nUID: `" + uid + "`\nTổng: " + String(numAuthorized) + " thẻ");
    } else {
      sendTelegram("⚠️ Thẻ mẫu `" + uid + "` đã tồn tại hoặc danh sách đầy!");
    }
  }

  // Button demo: xóa thẻ mẫu AABBCCDD
  else if (cmd == "/removecard_demo") {
    String uid = "AABBCCDD";
    if (removeCard(uid)) {
      sendTelegram("🗑️ *Đã xóa thẻ mẫu!*\nUID: `" + uid + "`\nCòn lại: " + String(numAuthorized) + " thẻ");
    } else {
      sendTelegram("❌ Không tìm thấy thẻ mẫu `" + uid + "` trong danh sách!");
    }
  }

  // Thêm thẻ qua text: /addcard XXXXXXXX
  else if (cmd.startsWith("/addcard ")) {
    String uid = cmd.substring(9);
    uid.trim(); uid.toUpperCase();
    if (uid.length() < 4) {
      sendTelegram("❌ UID không hợp lệ!\nVí dụ: /addcard A1B2C3D4");
    } else if (addCard(uid)) {
      sendTelegram("✅ *Thêm thẻ thành công!*\nUID: `" + uid + "`\nTổng: " + String(numAuthorized) + " thẻ");
    } else {
      sendTelegram("⚠️ Thẻ đã tồn tại hoặc danh sách đầy! (tối đa 10 thẻ)");
    }
  }

  // Xóa thẻ qua text: /removecard XXXXXXXX
  else if (cmd.startsWith("/removecard ")) {
    String uid = cmd.substring(12);
    uid.trim(); uid.toUpperCase();
    if (removeCard(uid)) {
      sendTelegram("🗑️ *Đã xóa thẻ!*\nUID: `" + uid + "`");
    } else {
      sendTelegram("❌ Không tìm thấy thẻ UID: `" + uid + "`");
    }
  }

  else if (cmd == "/help" || cmd == "/start" || cmd == "/menu") {
    sendMenuMain();
  }

  else if (cmd != "") {
    sendTelegram("❓ Lệnh không hợp lệ!\nGõ /menu để mở bảng điều khiển.");
  }
}

// ===================== POLL TELEGRAM =====================

void processTelegramUpdates() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  String url = API_BASE + "/getUpdates?timeout=1&offset=" + String(lastUpdateId + 1);
  http.begin(url);
  http.setTimeout(3000);
  int code = http.GET();
  if (code != 200) { http.end(); return; }

  String response = http.getString();
  http.end();

  DynamicJsonDocument doc(8192);
  if (deserializeJson(doc, response)) return;

  JsonArray results = doc["result"].as<JsonArray>();
  for (JsonObject update : results) {
    long updateId = update["update_id"].as<long>();
    if (updateId > lastUpdateId) lastUpdateId = updateId;

    // --- Xử lý tin nhắn text ---
    if (update.containsKey("message")) {
      String chatId = update["message"]["chat"]["id"].as<String>();
      if (chatId != CHAT_ID) continue;
      String text   = update["message"]["text"].as<String>();
      String sender = update["message"]["from"]["first_name"].as<String>();
      handleCommand(text, sender);
    }

    // --- Xử lý callback từ Inline Keyboard button ---
    else if (update.containsKey("callback_query")) {
      String chatId   = update["callback_query"]["message"]["chat"]["id"].as<String>();
      if (chatId != CHAT_ID) continue;
      String cbId     = update["callback_query"]["id"].as<String>();
      String data     = update["callback_query"]["data"].as<String>();
      String sender   = update["callback_query"]["from"]["first_name"].as<String>();
      handleCommand(data, sender, cbId);
    }
  }
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED,   OUTPUT);
  pinMode(BUZZER,    OUTPUT);
  digitalWrite(LED_RED, HIGH);

  SPI.begin();
  rfid.PCD_Init();
  lockServo.attach(SERVO_PIN);
  lockServo.write(0);

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("  DANG KHOI TAO ");
  lcd.setCursor(0, 1); lcd.print("   Xin cho...   ");

  WiFi.begin(ssid, password);
  Serial.print("Ket noi WiFi");
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500); Serial.print("."); attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK! IP: " + WiFi.localIP().toString());
    // Gửi thông báo khởi động + hiện menu button ngay
    sendTelegram("🚀 *Hệ thống Khóa Cửa Thông Minh đã khởi động!*\nIP: " + WiFi.localIP().toString());
    delay(500);
    sendMenuMain();
  }

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(" HE THONG KHOA  ");
  lcd.setCursor(0, 1); lcd.print(" Quet the de mo ");
  Serial.println("=== SAN SANG ===");
}

// ===================== LOOP =====================
void loop() {
  // Tự đóng cửa
  if (isDoorOpen && (millis() - doorOpenTime >= DOOR_TIMEOUT)) {
    closeDoor();
  }

  // Poll Telegram
  if (millis() - lastPollTime >= POLL_INTERVAL) {
    lastPollTime = millis();
    processTelegramUpdates();
  }

  // Đọc RFID
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = getCardUID();
  Serial.println("The quet: " + uid);

  if (isAuthorized(uid)) {
    openDoor(uid);
    sendTelegram("✅ *Thẻ hợp lệ!*\nUID: `" + uid + "`\nCửa đã mở. Tự đóng sau 5 giây.");
  } else {
    denyAccess(uid);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1000);
}