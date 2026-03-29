#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <time.h>

// =========================
// WiFi + Telegram config
// =========================
// Wokwi:
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Telegram Bot Token - lấy từ BotFather
const char* BOT_TOKEN = "8034409980:AAEGnnHqDMRA4ZwqmP1bArb-FDIgjBQmrDE";

// Để trống lần đầu chạy.
// Sau khi bạn nhắn /start cho bot, mở Serial Monitor để lấy chat_id.
// Sau đó dán vào đây để chỉ cho phép 1 tài khoản điều khiển.
const char* ALLOWED_CHAT_ID = "7999542154";

// =========================
// Stepper config
// =========================
static const uint8_t PIN_STEP = 26;
static const uint8_t PIN_DIR  = 27;
static const uint8_t PIN_EN   = 25;

// Số bước tương ứng rèm mở hoàn toàn
// Bạn cần tinh chỉnh theo cơ cấu kéo rèm của bạn
static const long CURTAIN_FULL_OPEN_STEPS = 4000;

// Tốc độ/ gia tốc
static const float MAX_SPEED     = 1200.0;
static const float ACCELERATION  = 700.0;

// =========================
// Global objects
// =========================
WiFiClientSecure securedClient;
UniversalTelegramBot bot(BOT_TOKEN, securedClient);
AccelStepper stepper(AccelStepper::DRIVER, PIN_STEP, PIN_DIR);

// Poll Telegram ngắn để stepper vẫn chạy mượt
unsigned long lastBotCheck = 0;
const unsigned long BOT_POLL_INTERVAL = 700;

// Vị trí quy ước
const long CLOSED_POS = 0;
const long OPEN_POS   = CURTAIN_FULL_OPEN_STEPS;

// Gửi thông báo khi tới đích
String activeChatId = "";
bool notifyWhenArrive = false;
bool wasMoving = false;

// =========================
// Helpers
// =========================
long clampLong(long value, long low, long high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

int getPositionPercent() {
  long pos = clampLong(stepper.currentPosition(), CLOSED_POS, OPEN_POS);
  return map(pos, CLOSED_POS, OPEN_POS, 0, 100);
}

String getCurtainState() {
  if (stepper.distanceToGo() != 0) return "Dang di chuyen";
  int percent = getPositionPercent();
  if (percent <= 2) return "Dang dong";
  if (percent >= 98) return "Dang mo";
  return "Dung o vi tri trung gian";
}

String buildStatusMessage() {
  String msg = "Trang thai rem:\n";
  msg += "- Vi tri: " + String(getPositionPercent()) + "%\n";
  msg += "- State: " + getCurtainState() + "\n";
  msg += "- Current steps: " + String(stepper.currentPosition()) + "\n";
  msg += "- Target steps: " + String(stepper.targetPosition());
  return msg;
}

void moveToPercent(int percent, const String& chatId) {
  percent = constrain(percent, 0, 100);
  long target = map(percent, 0, 100, CLOSED_POS, OPEN_POS);
  stepper.moveTo(target);
  activeChatId = chatId;
  notifyWhenArrive = true;
}

void sendHelp(const String& chatId) {
  String msg;
  msg += "Dieu khien rem cua bang Telegram\n\n";
  msg += "Lenh ho tro:\n";
  msg += "/open  - Mo rem 100%\n";
  msg += "/close - Dong rem 0%\n";
  msg += "/stop  - Dung mem tai vi tri hien tai\n";
  msg += "/status - Xem trang thai\n";
  msg += "/set <0..100> - Dua rem den phan tram mong muon\n\n";
  msg += "Vi du: /set 40";
  bot.sendMessage(chatId, msg, "");
}

void connectWiFi() {
  Serial.println("Dang ket noi WiFi...");
  WiFi.mode(WIFI_STA);

  // Tối ưu cho Wokwi
  if (String(WIFI_SSID) == "Wokwi-GUEST") {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void syncTimeForTLS() {
  Serial.print("Dong bo thoi gian NTP");
  configTime(0, 0, "pool.ntp.org");

  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    delay(200);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("\nTime synced.");
}

bool isAuthorizedChat(const String& chatId) {
  if (strlen(ALLOWED_CHAT_ID) == 0) {
    return true; // Cho phép mọi chat nếu chưa khoá
  }
  return chatId == String(ALLOWED_CHAT_ID);
}

void handleCommand(const String& chatId, String text, const String& fromName) {
  text.trim();

  Serial.println("==== Telegram message ====");
  Serial.println("From: " + fromName);
  Serial.println("chat_id: " + chatId);
  Serial.println("text: " + text);

  if (!isAuthorizedChat(chatId)) {
    bot.sendMessage(chatId, "Ban khong duoc phep dieu khien thiet bi nay.", "");
    return;
  }

  if (text == "/start" || text == "/help") {
    sendHelp(chatId);
    return;
  }

  if (text == "/open") {
    moveToPercent(100, chatId);
    bot.sendMessage(chatId, "Dang mo rem...", "");
    return;
  }

  if (text == "/close") {
    moveToPercent(0, chatId);
    bot.sendMessage(chatId, "Dang dong rem...", "");
    return;
  }

  if (text == "/stop") {
    stepper.stop(); // dung mem, co giam toc
    activeChatId = chatId;
    notifyWhenArrive = true;
    bot.sendMessage(chatId, "Dang dung rem...", "");
    return;
  }

  if (text == "/status") {
    bot.sendMessage(chatId, buildStatusMessage(), "");
    return;
  }

  if (text.startsWith("/set ")) {
    String value = text.substring(5);
    value.trim();
    int percent = value.toInt();

    if (percent < 0 || percent > 100) {
      bot.sendMessage(chatId, "Gia tri khong hop le. Hay dung /set <0..100>", "");
      return;
    }

    moveToPercent(percent, chatId);
    bot.sendMessage(chatId, "Dang dua rem den " + String(percent) + "%", "");
    return;
  }

  bot.sendMessage(chatId, "Lenh khong hop le. Gui /start de xem huong dan.", "");
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chatId = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String fromName = bot.messages[i].from_name;

    handleCommand(chatId, text, fromName);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nKhoi dong he thong rem cua...");

  pinMode(PIN_EN, OUTPUT);
  digitalWrite(PIN_EN, LOW); // A4988 ENABLE active-low => LOW = enable

  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCELERATION);

  // Quy ước khi khởi động: rèm đang đóng hoàn toàn
  // Ngoài thực tế nên thêm công tắc hành trình để homing
  stepper.setCurrentPosition(CLOSED_POS);

  connectWiFi();

  securedClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  syncTimeForTLS();

  bot.setMyCommands(
    "["
    "{\"command\":\"start\",\"description\":\"Xem huong dan\"},"
    "{\"command\":\"open\",\"description\":\"Mo rem\"},"
    "{\"command\":\"close\",\"description\":\"Dong rem\"},"
    "{\"command\":\"stop\",\"description\":\"Dung rem\"},"
    "{\"command\":\"status\",\"description\":\"Trang thai rem\"}"
    "]"
  );

  Serial.println("He thong san sang. Hay nhan /start tren Telegram.");
}

void loop() {
  // Quan trọng: phải gọi liên tục để stepper chạy mượt
  stepper.run();

  if (millis() - lastBotCheck > BOT_POLL_INTERVAL) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastBotCheck = millis();
  }

  bool isMoving = (stepper.distanceToGo() != 0);

  if (wasMoving && !isMoving && notifyWhenArrive && activeChatId.length() > 0) {
    bot.sendMessage(activeChatId, "Rem da den vi tri muc tieu.\n\n" + buildStatusMessage(), "");
    notifyWhenArrive = false;
  }

  wasMoving = isMoving;
}