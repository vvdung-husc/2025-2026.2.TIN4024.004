#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ============================================================
//   ⚙️  CẤU HÌNH - ĐIỀN THÔNG TIN CỦA BẠN VÀO ĐÂY
// ============================================================
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define BOT_TOKEN     "8760666671:AAHwglMFLQOfBl6qzn3nyrRbyIeVnA70v6k"       // 🤖 Token từ @BotFather
#define CHAT_ID       "-5173513541"         // 💬 Chat ID của Group
// ============================================================

// 📌 Chân GPIO
#define LED_PIN 23
#define PIR_PIN 27

// 🔄 Biến trạng thái
bool ledState        = false;
bool pirLastState    = false;
bool pirCurrentState = false;

// ⏱️ Thời gian kiểm tra Telegram (ms)
unsigned long lastBotCheck       = 0;
const int     BOT_CHECK_INTERVAL = 1000;  // 1 giây

// ⏱️ Thời gian chống nhiễu PIR (ms)
unsigned long lastPirTrigger = 0;
const int     PIR_COOLDOWN   = 5000;  // 5 giây

WiFiClientSecure      client;
UniversalTelegramBot  bot(BOT_TOKEN, client);

// ============================================================
//   🖨️  IN ĐƯỜNG KẺ PHÂN CÁCH RA SERIAL
// ============================================================
void printDivider() {
  Serial.println(F("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
}

// ============================================================
//   🎮  XỬ LÝ CÁC LỆNH TỪ TELEGRAM
// ============================================================
void handleCommand(String chat_id, String text, String from_name) {
  printDivider();
  Serial.println("  📩 LỆNH MỚI NHẬN ĐƯỢC");
  Serial.println("  👤 Từ    : " + from_name);
  Serial.println("  📝 Lệnh  : " + text);
  printDivider();

  // ----------------------------------------------------------
  if (text == "/start") {
    String welcome =
      "╔══════════════════════════╗\n"
      "║  🤖  ESP32 CONTROL BOT   ║\n"
      "╚══════════════════════════╝\n\n"
      "👋 Xin chào *" + from_name + "*!\n\n"
      "🟢 Hệ thống đang *hoạt động bình thường*.\n\n"
      "📋 *DANH SÁCH LỆNH:*\n"
      "┌─────────────────────────────\n"
      "│ 💡 /led2\\_on   → Bật LED đỏ\n"
      "│ 🔌 /led2\\_off  → Tắt LED đỏ\n"
      "│ 📊 /get\\_state → Xem trạng thái LED\n"
      "└─────────────────────────────\n\n"
      "📡 Bot sẽ *tự động cảnh báo* khi phát hiện chuyển động.";
    bot.sendMessage(chat_id, welcome, "Markdown");

    Serial.println("  ✅ Đã gửi tin nhắn chào mừng tới: " + from_name);
    printDivider();

  // ----------------------------------------------------------
  } else if (text == "/led2_on") {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);

    String msg =
      "💡 *LED ĐÃ BẬT*\n"
      "━━━━━━━━━━━━━━━━\n"
      "📍 Chân GPIO : 23\n"
      "🔆 Trạng thái: *ON* (Đang sáng)\n"
      "👤 Thực hiện bởi: " + from_name;
    bot.sendMessage(chat_id, msg, "Markdown");

    Serial.println("  💡 LED  →  BẬT (ON)");
    Serial.println("  👤 Lệnh từ: " + from_name);
    printDivider();

  // ----------------------------------------------------------
  } else if (text == "/led2_off") {
    ledState = false;
    digitalWrite(LED_PIN, LOW);

    String msg =
      "🔌 *LED ĐÃ TẮT*\n"
      "━━━━━━━━━━━━━━━━\n"
      "📍 Chân GPIO : 23\n"
      "🌑 Trạng thái: *OFF* (Đã tắt)\n"
      "👤 Thực hiện bởi: " + from_name;
    bot.sendMessage(chat_id, msg, "Markdown");

    Serial.println("  🔌 LED  →  TẮT (OFF)");
    Serial.println("  👤 Lệnh từ: " + from_name);
    printDivider();

  // ----------------------------------------------------------
  } else if (text == "/get_state") {
    String stateEmoji  = ledState ? "💡" : "🌑";
    String stateText   = ledState ? "*BẬT (ON)*"   : "*TẮT (OFF)*";
    String stateSerial = ledState ? "BẬT (ON)"     : "TẮT (OFF)";

    String msg =
      "📊 *TRẠNG THÁI THIẾT BỊ*\n"
      "━━━━━━━━━━━━━━━━━━━━━━━\n"
      + stateEmoji + " LED      : " + stateText + "\n"
      "📡 PIR Sensor : Đang theo dõi\n"
      "🟢 Hệ thống  : Hoạt động bình thường";
    bot.sendMessage(chat_id, msg, "Markdown");

    Serial.println("  📊 Truy vấn trạng thái:");
    Serial.println("     " + stateEmoji + "  LED: " + stateSerial);
    Serial.println("     📡 PIR: Đang hoạt động");
    printDivider();

  // ----------------------------------------------------------
  } else {
    bot.sendMessage(chat_id,
      "⚠️ *Lệnh không hợp lệ!*\n"
      "Gõ /start để xem danh sách lệnh.",
      "Markdown");

    Serial.println("  ⚠️  Lệnh không xác định: " + text);
    printDivider();
  }
}

// ============================================================
//   🚀  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("╔════════════════════════════════════════╗"));
  Serial.println(F("║   🤖  ESP32 TELEGRAM BOT  🤖           ║"));
  Serial.println(F("║       PIR Motion Sensor Alert          ║"));
  Serial.println(F("╚════════════════════════════════════════╝"));
  Serial.println();

  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println(F("  ✅ GPIO khởi tạo thành công"));
  Serial.println(F("     📍 LED  → Pin 23"));
  Serial.println(F("     📍 PIR  → Pin 27"));
  printDivider();

  // 📶 Kết nối WiFi
  Serial.print(F("  📶 Đang kết nối WiFi"));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println(F("  ✅ WiFi kết nối thành công!"));
  Serial.print(F("  🌐 Địa chỉ IP : "));
  Serial.println(WiFi.localIP());
  printDivider();

  // 📨 Gửi thông báo khởi động lên Telegram
  Serial.println(F("  📨 Đang gửi thông báo khởi động lên Telegram..."));

  bot.sendMessage(CHAT_ID,
    "🚀 *HỆ THỐNG KHỞI ĐỘNG*\n"
    "━━━━━━━━━━━━━━━━━━━━━━━\n"
    "✅ ESP32 đã kết nối thành công!\n"
    "📡 PIR Sensor đang theo dõi...\n"
    "💡 LED mặc định : *TẮT (OFF)*\n\n"
    "📋 Gõ /start để xem danh sách lệnh.",
    "Markdown");

  Serial.println(F("  ✅ Thông báo khởi động đã gửi thành công!"));
  printDivider();
  Serial.println(F("  🟢 Hệ thống sẵn sàng hoạt động!"));
  printDivider();
}

// ============================================================
//   🔁  LOOP
// ============================================================
void loop() {

  // ── 📡 Kiểm tra PIR ────────────────────────────────────────
  pirCurrentState = digitalRead(PIR_PIN);

  if (pirCurrentState == HIGH && pirLastState == LOW) {
    unsigned long now = millis();
    if (now - lastPirTrigger > PIR_COOLDOWN) {
      lastPirTrigger = now;

      printDivider();
      Serial.println(F("  🚨 CẢNH BÁO CHUYỂN ĐỘNG!"));
      Serial.println(F("  📡 PIR Sensor đã phát hiện có chuyển động!"));
      Serial.println(F("  📨 Đang gửi cảnh báo lên Telegram..."));
      printDivider();

      bot.sendMessage(CHAT_ID,
        "🚨 *CẢNH BÁO CHUYỂN ĐỘNG!*\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "📡 PIR Sensor phát hiện *có người/vật di chuyển*!\n"
        "📍 Vị trí       : Khu vực giám sát\n"
        "🔴 Mức độ      : Cần chú ý\n"
        "⏱️ Hệ thống đang tiếp tục theo dõi...",
        "Markdown");
    }
  }
  pirLastState = pirCurrentState;

  // ── 🤖 Kiểm tra lệnh Telegram ─────────────────────────────
  if (millis() - lastBotCheck > BOT_CHECK_INTERVAL) {
    lastBotCheck = millis();
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages > 0) {
      for (int i = 0; i < numNewMessages; i++) {
        String chat_id   = String(bot.messages[i].chat_id);
        String text      = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;

        // Chỉ xử lý lệnh từ group đã cấu hình
        if (chat_id == CHAT_ID) {
          handleCommand(chat_id, text, from_name);
        } else {
          printDivider();
          Serial.println("  ⚠️  Tin nhắn từ chat lạ bị bỏ qua!");
          Serial.println("  🆔 Chat ID: " + chat_id);
          printDivider();
        }
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}