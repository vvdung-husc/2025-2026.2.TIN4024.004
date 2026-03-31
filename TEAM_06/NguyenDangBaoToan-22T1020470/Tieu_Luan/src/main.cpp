#include <Arduino.h>
#include <ESP32Servo.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <SPI.h>
#include <WebServer.h>
#include <WiFi.h>

#include <vector>

#ifndef WIFI_STA_SSID_VALUE
#define WIFI_STA_SSID_VALUE ""
#endif

#ifndef WIFI_STA_PASSWORD_VALUE
#define WIFI_STA_PASSWORD_VALUE ""
#endif

#ifndef WIFI_STA_CHANNEL_VALUE
#define WIFI_STA_CHANNEL_VALUE 0
#endif

namespace {

constexpr uint8_t RFID_SS_PIN = 5;
constexpr uint8_t RFID_SCK_PIN = 18;
constexpr uint8_t RFID_MOSI_PIN = 23;
constexpr uint8_t RFID_MISO_PIN = 19;
constexpr uint8_t RFID_RST_PIN = 21;

constexpr uint8_t SERVO_PIN = 13;
constexpr uint8_t BUZZER_PIN = 14;
constexpr uint8_t RED_LED_PIN = 25;
constexpr uint8_t GREEN_LED_PIN = 26;
constexpr uint8_t RELAY_PIN = 27;

constexpr int SERVO_LOCK_ANGLE = 0;
constexpr int SERVO_UNLOCK_ANGLE = 90;
constexpr bool RELAY_ACTIVE_HIGH = true;
constexpr unsigned long UNLOCK_DURATION_MS = 5000;
constexpr unsigned long CARD_DEBOUNCE_MS = 1500;
constexpr unsigned long BOOT_HOLD_GRACE_MS = 3000;
constexpr unsigned long BUZZER_TONE_SETTLE_MS = 20;
constexpr unsigned long WIFI_STA_CONNECT_TIMEOUT_MS = 15000;
constexpr unsigned long WIFI_STA_RETRY_INTERVAL_MS = 10000;

// Configure STA through PlatformIO build flags.
constexpr const char *WIFI_STA_SSID = WIFI_STA_SSID_VALUE;
constexpr const char *WIFI_STA_PASSWORD = WIFI_STA_PASSWORD_VALUE;
constexpr int WIFI_STA_CHANNEL = WIFI_STA_CHANNEL_VALUE;
constexpr const char *WIFI_AP_SSID = "ESP32-DoorLock";
constexpr const char *WIFI_AP_PASSWORD = "12345678";

constexpr const char *WEB_USERNAME = "admin";
constexpr const char *WEB_PASSWORD = "123456";

constexpr const char *PREF_NAMESPACE = "doorlock";
constexpr const char *PREF_KEY_CARDS = "cards";

constexpr uint8_t BUZZER_CHANNEL = 15;

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Servo doorServo;
Preferences preferences;
WebServer server(80);
std::vector<String> authorizedCards;

String lastScannedUid = "Chưa có";
String lastEventMessage = "Hệ thống sẵn sàng";
String wifiModeText = "Chưa kết nối";
String wifiAddressText = "-";
String lastProcessedUid = "";

bool doorLocked = true;
bool firstCardMode = false;
bool wifiStaConnecting = false;
bool wifiStaWasConnected = false;
unsigned long unlockDeadline = 0;
unsigned long lastProcessedAt = 0;
unsigned long bootStartedAt = 0;
unsigned long wifiStaAttemptStartedAt = 0;
unsigned long wifiStaLastAttemptAt = 0;
bool bootHoldConsumed = false;

String normalizeUid(String uid) {
  uid.trim();
  uid.toUpperCase();

  String normalized;
  bool lastWasSpace = true;

  for (size_t i = 0; i < uid.length(); ++i) {
    const char ch = uid[i];
    if (isxdigit(static_cast<unsigned char>(ch))) {
      normalized += ch;
      lastWasSpace = false;
    } else if ((ch == ' ' || ch == ':' || ch == '-') && !lastWasSpace && !normalized.isEmpty()) {
      normalized += ' ';
      lastWasSpace = true;
    }
  }

  normalized.trim();
  return normalized;
}

String uidToString(const MFRC522::Uid &uid) {
  String value;

  for (byte i = 0; i < uid.size; i++) {
    if (i > 0) {
      value += ' ';
    }

    if (uid.uidByte[i] < 0x10) {
      value += '0';
    }
    value += String(uid.uidByte[i], HEX);
  }

  value.toUpperCase();
  return value;
}

String escapeHtml(const String &text) {
  String escaped;
  escaped.reserve(text.length() + 16);

  for (size_t i = 0; i < text.length(); ++i) {
    const char ch = text[i];
    switch (ch) {
      case '&':
        escaped += F("&amp;");
        break;
      case '<':
        escaped += F("&lt;");
        break;
      case '>':
        escaped += F("&gt;");
        break;
      case '"':
        escaped += F("&quot;");
        break;
      default:
        escaped += ch;
        break;
    }
  }

  return escaped;
}

void setRelay(bool unlocked) {
  const bool activeState = RELAY_ACTIVE_HIGH ? HIGH : LOW;
  const bool inactiveState = RELAY_ACTIVE_HIGH ? LOW : HIGH;
  digitalWrite(RELAY_PIN, unlocked ? activeState : inactiveState);
}

void setStatusLeds(bool unlocked) {
  digitalWrite(GREEN_LED_PIN, unlocked ? HIGH : LOW);
  digitalWrite(RED_LED_PIN, unlocked ? LOW : HIGH);
}

void beginBuzzer() {
  setToneChannel(BUZZER_CHANNEL);
  noTone(BUZZER_PIN);
  digitalWrite(BUZZER_PIN, LOW);
}

void buzzerTone(unsigned int frequency, unsigned long durationMs) {
  tone(BUZZER_PIN, frequency, durationMs);
  delay(durationMs + BUZZER_TONE_SETTLE_MS);
  noTone(BUZZER_PIN);
  digitalWrite(BUZZER_PIN, LOW);
}

void playSuccessTone() {
  buzzerTone(2200, 120);
  delay(40);
  buzzerTone(2800, 140);
}

void playCardAcceptedTone() {
  buzzerTone(2400, 90);
}

void playErrorTone() {
  for (int i = 0; i < 2; ++i) {
    buzzerTone(700, 180);
    delay(50);
  }
}

void saveAuthorizedCards() {
  String payload;

  for (size_t i = 0; i < authorizedCards.size(); ++i) {
    if (i > 0) {
      payload += ';';
    }
    payload += authorizedCards[i];
  }

  preferences.putString(PREF_KEY_CARDS, payload);
}

void loadAuthorizedCards() {
  authorizedCards.clear();

  const String payload = preferences.getString(PREF_KEY_CARDS, "");
  if (payload.isEmpty()) {
    firstCardMode = true;
    lastEventMessage = "Quét thẻ đầu tiên để đăng ký";
    return;
  }

  int startIndex = 0;
  while (startIndex < payload.length()) {
    int separatorIndex = payload.indexOf(';', startIndex);
    if (separatorIndex < 0) {
      separatorIndex = payload.length();
    }

    const String uid = normalizeUid(payload.substring(startIndex, separatorIndex));
    if (!uid.isEmpty()) {
      authorizedCards.push_back(uid);
    }
    startIndex = separatorIndex + 1;
  }

  firstCardMode = authorizedCards.empty();
}

bool isAuthorizedCard(const String &uid) {
  for (const String &savedUid : authorizedCards) {
    if (savedUid == uid) {
      return true;
    }
  }
  return false;
}

bool addAuthorizedCard(String uid) {
  uid = normalizeUid(uid);
  if (uid.isEmpty() || isAuthorizedCard(uid)) {
    return false;
  }

  authorizedCards.push_back(uid);
  saveAuthorizedCards();
  firstCardMode = false;
  return true;
}

bool removeAuthorizedCard(String uid) {
  uid = normalizeUid(uid);

  for (auto it = authorizedCards.begin(); it != authorizedCards.end(); ++it) {
    if (*it == uid) {
      authorizedCards.erase(it);
      saveAuthorizedCards();
      firstCardMode = authorizedCards.empty();
      return true;
    }
  }

  return false;
}

void lockDoor(const String &reason) {
  doorServo.write(SERVO_LOCK_ANGLE);
  setRelay(false);
  setStatusLeds(false);
  doorLocked = true;
  unlockDeadline = 0;

  if (!reason.isEmpty()) {
    lastEventMessage = "Cửa đã khóa: " + reason;
  }

  Serial.println(lastEventMessage);
}

void unlockDoor(const String &reason) {
  doorServo.write(SERVO_UNLOCK_ANGLE);
  setRelay(true);
  setStatusLeds(true);
  doorLocked = false;
  unlockDeadline = millis() + UNLOCK_DURATION_MS;
  lastEventMessage = "Cửa đã mở: " + reason;

  Serial.println(lastEventMessage);
}

void denyAccess(const String &uid) {
  lastEventMessage = "Từ chối thẻ: " + uid;
  Serial.println(lastEventMessage);
  playErrorTone();

  digitalWrite(RED_LED_PIN, LOW);
  delay(120);
  digitalWrite(RED_LED_PIN, HIGH);
  delay(120);
}

String jsonEscape(const String &text) {
  String escaped;
  escaped.reserve(text.length() + 16);

  for (size_t i = 0; i < text.length(); ++i) {
    const char ch = text[i];
    switch (ch) {
      case '\\':
        escaped += F("\\\\");
        break;
      case '"':
        escaped += F("\\\"");
        break;
      case '\n':
        escaped += F("\\n");
        break;
      case '\r':
        break;
      default:
        escaped += ch;
        break;
    }
  }

  return escaped;
}

bool ensureAuthenticated() {
  if (server.authenticate(WEB_USERNAME, WEB_PASSWORD)) {
    return true;
  }

  server.requestAuthentication();
  return false;
}

void redirectHome() {
  server.sendHeader("Location", "/");
  server.send(303);
}

String buildHtmlPage() {
  String page;
  page.reserve(3600);

  page += F(
      "<!DOCTYPE html><html><head><meta charset='utf-8'>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>ESP32 Smart Door Lock</title>"
      "<style>"
      "body{font-family:Arial,sans-serif;background:#f4f6f8;color:#1f2937;margin:0;padding:24px;}"
      ".wrap{max-width:860px;margin:0 auto;background:#fff;border-radius:18px;padding:24px;"
      "box-shadow:0 10px 30px rgba(15,23,42,.08);}"
      "h1{margin-top:0;} .status{padding:14px 16px;border-radius:12px;margin:16px 0;background:#eef2ff;}"
      ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px;}"
      ".card{background:#f8fafc;border:1px solid #e5e7eb;border-radius:14px;padding:14px;}"
      "button{padding:10px 14px;border:none;border-radius:10px;cursor:pointer;font-weight:700;}"
      ".open{background:#16a34a;color:#fff;} .close{background:#dc2626;color:#fff;}"
      ".add{background:#2563eb;color:#fff;} .delete{background:#6b7280;color:#fff;}"
      "form{margin:10px 0;} input{width:100%;padding:10px 12px;border:1px solid #cbd5e1;border-radius:10px;}"
      "table{width:100%;border-collapse:collapse;margin-top:8px;} td,th{padding:10px;border-bottom:1px solid #e5e7eb;}"
      "small{color:#64748b;} a{color:#2563eb;text-decoration:none;}"
      "</style></head><body><div class='wrap'>");

  page += F("<h1>Khóa Cửa Thông Minh ESP32</h1>");
  page += F("<div class='status'><strong>Trạng thái cửa:</strong> ");
  page += doorLocked ? F("Đang khóa") : F("Đang mở");
  page += F("<br><strong>Chế độ WiFi:</strong> ");
  page += escapeHtml(wifiModeText);
  page += F("<br><strong>Địa chỉ:</strong> ");
  page += escapeHtml(wifiAddressText);
  page += F("<br><strong>Sự kiện cuối:</strong> ");
  page += escapeHtml(lastEventMessage);
  page += F("<br><strong>Thẻ gần nhất:</strong> ");
  page += escapeHtml(lastScannedUid);
  page += F("</div>");

  page += F("<div class='grid'>");
  page += F("<div class='card'><h3>Điều khiển cửa</h3>");
  page += F("<form action='/unlock' method='post'><button class='open' type='submit'>Mở cửa</button></form>");
  page += F("<form action='/lock' method='post'><button class='close' type='submit'>Khóa cửa</button></form>");
  page += F("<small>Đăng nhập với tài khoản admin/123456.</small></div>");

  page += F("<div class='card'><h3>Thẻ được phép</h3>");
  page += F("<form action='/add' method='post'>");
  page += F("<input name='uid' placeholder='AA BB CC DD'>");
  page += F("<button class='add' type='submit'>Thêm UID</button></form>");
  page += F("<form action='/enroll-last' method='post'><button class='add' type='submit'>Thêm thẻ vừa quét</button></form>");
  page += F("<small>Nếu chưa có thẻ nào, thẻ đầu tiên quét vào sẽ được đăng ký tự động.</small></div>");
  page += F("</div>");

  page += F("<div class='card'><h3>Danh sách UID</h3><table><tr><th>UID</th><th>Hành động</th></tr>");
  if (authorizedCards.empty()) {
    page += F("<tr><td colspan='2'>Chưa có UID nào được lưu.</td></tr>");
  } else {
    for (const String &uid : authorizedCards) {
      page += F("<tr><td>");
      page += escapeHtml(uid);
      page += F("</td><td>");
      page += F("<form action='/delete' method='post'>");
      page += F("<input type='hidden' name='uid' value='");
      page += escapeHtml(uid);
      page += F("'>");
      page += F("<button class='delete' type='submit'>Xóa</button></form></td></tr>");
    }
  }
  page += F("</table><small>Xem API JSON tại <a href='/status'>/status</a>.</small></div>");

  page += F("</div></body></html>");
  return page;
}

void handleRoot() {
  if (!ensureAuthenticated()) {
    return;
  }

  server.send(200, "text/html", buildHtmlPage());
}

void handleUnlock() {
  if (!ensureAuthenticated()) {
    return;
  }

  playSuccessTone();
  unlockDoor("WiFi");
  redirectHome();
}

void handleLock() {
  if (!ensureAuthenticated()) {
    return;
  }

  lockDoor("WiFi");
  redirectHome();
}

void handleAddCard() {
  if (!ensureAuthenticated()) {
    return;
  }

  const String uid = normalizeUid(server.arg("uid"));
  if (addAuthorizedCard(uid)) {
    lastEventMessage = "Đã thêm UID: " + uid;
  } else {
    lastEventMessage = "Không thể thêm UID";
  }
  redirectHome();
}

void handleEnrollLast() {
  if (!ensureAuthenticated()) {
    return;
  }

  const String uid = normalizeUid(lastScannedUid);
  if (addAuthorizedCard(uid)) {
    lastEventMessage = "Đã thêm thẻ vừa quét: " + uid;
  } else {
    lastEventMessage = "Không thể thêm thẻ vừa quét";
  }
  redirectHome();
}

void handleDeleteCard() {
  if (!ensureAuthenticated()) {
    return;
  }

  const String uid = normalizeUid(server.arg("uid"));
  if (removeAuthorizedCard(uid)) {
    lastEventMessage = "Đã xóa UID: " + uid;
  } else {
    lastEventMessage = "Không tìm thấy UID để xóa";
  }
  redirectHome();
}

void handleStatus() {
  if (!ensureAuthenticated()) {
    return;
  }

  String payload = "{";
  payload += "\"door\":\"";
  payload += doorLocked ? "locked" : "unlocked";
  payload += "\",\"wifiMode\":\"";
  payload += jsonEscape(wifiModeText);
  payload += "\",\"ip\":\"";
  payload += jsonEscape(wifiAddressText);
  payload += "\",\"lastUid\":\"";
  payload += jsonEscape(lastScannedUid);
  payload += "\",\"lastEvent\":\"";
  payload += jsonEscape(lastEventMessage);
  payload += "\",\"authorizedCount\":";
  payload += String(authorizedCards.size());
  payload += "}";

  server.send(200, "application/json", payload);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/unlock", HTTP_POST, handleUnlock);
  server.on("/lock", HTTP_POST, handleLock);
  server.on("/add", HTTP_POST, handleAddCard);
  server.on("/enroll-last", HTTP_POST, handleEnrollLast);
  server.on("/delete", HTTP_POST, handleDeleteCard);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();
}

bool hasWiFiStationConfig() {
  return strlen(WIFI_STA_SSID) > 0;
}

void updateWiFiStatus() {
  const bool staConfigured = hasWiFiStationConfig();
  const bool staConnected = staConfigured && WiFi.status() == WL_CONNECTED;

  if (!staConfigured) {
    wifiModeText = "AP";
  } else if (staConnected) {
    wifiModeText = "AP+STA";
  } else if (wifiStaConnecting) {
    wifiModeText = "AP+STA (đang kết nối)";
  } else {
    wifiModeText = "AP+STA (đang thử lại)";
  }

  wifiAddressText = "AP: " + WiFi.softAPIP().toString();
  if (staConfigured) {
    wifiAddressText += " | STA: ";
    wifiAddressText += staConnected ? WiFi.localIP().toString() : "-";
  }
}

void startWiFiAccessPoint() {
  WiFi.mode(hasWiFiStationConfig() ? WIFI_AP_STA : WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  updateWiFiStatus();

  Serial.println("WiFi AP ready");
  Serial.print("SSID: ");
  Serial.println(WIFI_AP_SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
}

void beginWiFiStationConnection() {
  if (!hasWiFiStationConfig()) {
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  if (WIFI_STA_CHANNEL > 0) {
    WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD, WIFI_STA_CHANNEL);
  } else {
    WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);
  }
  wifiStaConnecting = true;
  wifiStaAttemptStartedAt = millis();
  wifiStaLastAttemptAt = wifiStaAttemptStartedAt;
  updateWiFiStatus();

  Serial.print("Dang ket noi WiFi STA: ");
  Serial.println(WIFI_STA_SSID);
  if (WIFI_STA_CHANNEL > 0) {
    Serial.print("Kenh WiFi: ");
    Serial.println(WIFI_STA_CHANNEL);
  }
}

void connectWiFi() {
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  startWiFiAccessPoint();

  if (hasWiFiStationConfig()) {
    beginWiFiStationConnection();
  }
}

void maintainWiFi() {
  if (!hasWiFiStationConfig()) {
    updateWiFiStatus();
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiStaConnecting = false;

    if (!wifiStaWasConnected) {
      wifiStaWasConnected = true;
      updateWiFiStatus();
      Serial.println("WiFi STA connected");
      Serial.print("STA IP: ");
      Serial.println(WiFi.localIP());
      return;
    }

    updateWiFiStatus();
    return;
  }

  if (wifiStaWasConnected) {
    wifiStaWasConnected = false;
    Serial.println("WiFi STA disconnected");
  }

  if (wifiStaConnecting &&
      millis() - wifiStaAttemptStartedAt >= WIFI_STA_CONNECT_TIMEOUT_MS) {
    wifiStaConnecting = false;
    wifiStaLastAttemptAt = millis();
    Serial.println("WiFi STA timeout, se thu lai sau");
  }

  updateWiFiStatus();

  if (!wifiStaConnecting &&
      millis() - wifiStaLastAttemptAt >= WIFI_STA_RETRY_INTERVAL_MS) {
    beginWiFiStationConnection();
  }
}

void processCardScan() {
  bool cardReady = false;

  if (rfid.PICC_IsNewCardPresent()) {
    cardReady = rfid.PICC_ReadCardSerial();
  } else if (!bootHoldConsumed &&
             millis() - bootStartedAt <= BOOT_HOLD_GRACE_MS) {
    // Support Wokwi's Hold switch when the card is already present during boot.
    cardReady = rfid.PICC_ReadCardSerial();
    if (cardReady) {
      bootHoldConsumed = true;
    }
  }

  if (!cardReady) {
    return;
  }

  const String uid = uidToString(rfid.uid);
  const unsigned long now = millis();

  if (uid == lastProcessedUid && now - lastProcessedAt < CARD_DEBOUNCE_MS) {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  lastProcessedUid = uid;
  lastProcessedAt = now;
  lastScannedUid = uid;
  Serial.print("RFID UID: ");
  Serial.println(uid);

  if (firstCardMode) {
    addAuthorizedCard(uid);
    lastEventMessage = "Đã đăng ký thẻ đầu tiên: " + uid;
    playCardAcceptedTone();
    unlockDoor("thẻ đầu tiên");
  } else if (isAuthorizedCard(uid)) {
    playCardAcceptedTone();
    unlockDoor("RFID hợp lệ");
  } else {
    denyAccess(uid);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void setupHardware() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  beginBuzzer();

  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 500, 2400);

  lockDoor("khởi động");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  bootStartedAt = millis();
  Serial.println();
  Serial.println("ESP32 Smart Door Lock");

  preferences.begin(PREF_NAMESPACE, false);
  loadAuthorizedCards();

  setupHardware();

  SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
  rfid.PCD_Init();

  connectWiFi();
  setupServer();

  if (firstCardMode) {
    lastEventMessage = "Quét thẻ đầu tiên để đăng ký";
  }

  Serial.println("Tai khoan web: admin / 123456");
  Serial.println(lastEventMessage);
}

void loop() {
  server.handleClient();
  maintainWiFi();
  processCardScan();

  if (!doorLocked && unlockDeadline != 0 &&
      static_cast<long>(millis() - unlockDeadline) >= 0) {
    lockDoor("tự động hết thời gian");
  }
}
