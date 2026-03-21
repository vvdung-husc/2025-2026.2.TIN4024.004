/*
  Nhom 6
  1. Nguyen Huu Trinh - 22T1020481
  2. Ho Sy Vinh - 22T1020521
  3. Nguyễn Đăng Bảo Toàn -22T1020470



*/

#define BLYNK_TEMPLATE_ID   " "// ae ai làm được cái blynk thì điền hộ vào đống này với
#define BLYNK_TEMPLATE_NAME " "// ae ai làm được cái blynk thì điền hộ vào đống này với
#define BLYNK_AUTH_TOKEN    " "// ae ai làm được cái blynk thì điền hộ vào đống này với

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= TELEGRAM =================
// Hay thay bang token moi sau khi ban regenerate token
const char* BOT_TOKEN = "8645266510:AAE9yYZFO0G-scxNUms3ViTluuwUnxFRFwc";

// De rong o lan chay dau tien de lay chat_id tu Serial Monitor
const char* CHAT_ID = "";

// ================= THONG TIN NHOM =================
const char* GROUP_INFO = "IoT-Nhóm 2-Team 6";

// ================= PIN ESP32 =================
#define DHTPIN      15
#define DHTTYPE     DHT22
#define RELAY_PIN   16
#define MQ2_PIN     32
#define OLED_SDA    13
#define OLED_SCL    12
#define OLED_ADDR   0x3C

// ================= CHE DO MQ2 =================
// 0 = doc cam bien that / Wokwi
// 1 = sinh ngau nhien neu khong co cam bien
#define USE_FAKE_MQ2 0

// ================= CAU HINH CHUNG =================
#define TELEGRAM_POLL_MS     2000UL
#define SENSOR_UPDATE_MS     2000UL
#define RECONNECT_CHECK_MS   5000UL
#define GAS_ALERT_THRESHOLD  700
#define GAS_ALERT_COOLDOWN   60000UL

// ================= DOI TUONG =================
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
BlynkTimer timer;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ================= BIEN TOAN CUC =================
float temperature = 0.0f;
float humidity = 0.0f;
int gasValue = 0;
bool relayState = false;

unsigned long lastTelegramPoll = 0;
unsigned long lastGasAlertMs = 0;
bool gasAlertLatched = false;

// ================= HAM HO TRO =================
String formatUptime(unsigned long ms) {
  unsigned long totalSeconds = ms / 1000;
  unsigned long days = totalSeconds / 86400;
  unsigned long hours = (totalSeconds % 86400) / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;

  char buf[24];
  if (days > 0) {
    snprintf(buf, sizeof(buf), "%lud %02lu:%02lu:%02lu", days, hours, minutes, seconds);
  } else {
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", hours, minutes, seconds);
  }
  return String(buf);
}

String extractCommand(String text) {
  text.trim();

  int spacePos = text.indexOf(' ');
  if (spacePos > 0) {
    text = text.substring(0, spacePos);
  }

  int atPos = text.indexOf('@');
  if (atPos > 0) {
    text = text.substring(0, atPos);
  }

  text.toLowerCase();
  return text;
}

void setRelay(bool state, bool pushToBlynk = true) {
  relayState = state;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);

  if (pushToBlynk && Blynk.connected()) {
    Blynk.virtualWrite(V1, relayState ? 1 : 0);
  }
}

String sensorMessage() {
  String msg = "THONG TIN CAM BIEN\n";
  msg += "Nhiet do: " + String(temperature, 1) + " C\n";
  msg += "Do am: " + String(humidity, 1) + " %\n";
  msg += "Khi ga: " + String(gasValue);
  return msg;
}

// ================= WIFI =================
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Dang ket noi WiFi");
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nKhong ket noi duoc WiFi");
  }
}

void ensureConnections() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi mat ket noi -> dang thu lai...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    return;
  }

  if (!Blynk.connected()) {
    Serial.println("Blynk mat ket noi -> dang thu lai...");
    Blynk.connect(1000);
  }
}

// ================= DOC CAM BIEN =================
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity = h;

#if USE_FAKE_MQ2
  gasValue = random(350, 901);
#else
  gasValue = analogRead(MQ2_PIN);

  if (gasValue < 0 || gasValue > 4095) {
    gasValue = random(350, 901);
  }
#endif
}

// ================= OLED =================
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("NHOM 6 - ESP32");
  display.printf("T: %.1f C\n", temperature);
  display.printf("H: %.1f %%\n", humidity);
  display.printf("Gas: %d\n", gasValue);
  display.printf("LED: %s\n", relayState ? "ON" : "OFF");
  display.printf("Up: %s\n", formatUptime(millis()).c_str());
  display.printf("WiFi:%s B:%s",
                 WiFi.status() == WL_CONNECTED ? "OK" : "NO",
                 Blynk.connected() ? "OK" : "NO");

  display.display();
}

// ================= BLYNK =================
BLYNK_WRITE(V1) {
  setRelay(param.asInt() == 1, false);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
  Blynk.virtualWrite(V5, GROUP_INFO);
}

void updateBlynk() {
  if (!Blynk.connected()) return;

  Blynk.virtualWrite(V0, formatUptime(millis()));
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, gasValue);
  Blynk.virtualWrite(V5, GROUP_INFO);
}

// ================= TELEGRAM =================
void handleTelegram(int n) {
  for (int i = 0; i < n; i++) {
    String rawText = bot.messages[i].text;
    String command = extractCommand(rawText);
    String chat_id = String(bot.messages[i].chat_id);

    Serial.print("Telegram chat_id: ");
    Serial.print(chat_id);
    Serial.print(" | text: ");
    Serial.println(rawText);

    // Neu da cau hinh CHAT_ID, chi xu ly dung nhom do
    if (strlen(CHAT_ID) > 0 && chat_id != String(CHAT_ID)) {
      bot.sendMessage(chat_id, "Bot chi xu ly lenh tu nhom da cau hinh.", "");
      continue;
    }

    if (command == "/start") {
      String welcome = "Chao mung den voi Nhom 6!\n";
      welcome += "/led_on - Bat LED\n";
      welcome += "/led_off - Tat LED\n";
      welcome += "/led_status - Xem trang thai LED\n";
      welcome += "/get_weather - Xem nhiet do va do am hien tai";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (command == "/led_on") {
      setRelay(true, true);
      bot.sendMessage(chat_id, "Da bat LED", "");
    }
    else if (command == "/led_off") {
      setRelay(false, true);
      bot.sendMessage(chat_id, "Da tat LED", "");
    }
    else if (command == "/led_status") {
      bot.sendMessage(chat_id, relayState ? "LED dang ON" : "LED dang OFF", "");
    }
    else if (command == "/get_weather") {
      bot.sendMessage(chat_id, sensorMessage(), "");
    }
    else {
      bot.sendMessage(chat_id, "Lenh khong hop le. Gui /start de xem danh sach lenh.", "");
    }
  }
}

void checkTelegram() {
  if (millis() - lastTelegramPoll < TELEGRAM_POLL_MS) return;

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages > 0) {
    handleTelegram(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  lastTelegramPoll = millis();
}

// ================= CANH BAO GAS =================
void notifyGasAlert() {
  if (strlen(CHAT_ID) == 0) return;

  if (gasValue >= GAS_ALERT_THRESHOLD) {
    if (!gasAlertLatched || millis() - lastGasAlertMs > GAS_ALERT_COOLDOWN) {
      bot.sendMessage(String(CHAT_ID), "CANH BAO: Muc khi ga cao = " + String(gasValue), "");
      gasAlertLatched = true;
      lastGasAlertMs = millis();
    }
  }
  else if (gasValue < GAS_ALERT_THRESHOLD - 50) {
    gasAlertLatched = false;
  }
}

// ================= TASK CHINH =================
void taskAll() {
  readSensors();
  updateOLED();
  updateBlynk();
  notifyGasAlert();
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  randomSeed(micros());
  analogReadResolution(12);

  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false, false);

  dht.begin();
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Khong tim thay OLED SSD1306!");
    while (true) {
      delay(10);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Khoi dong he thong...");
  display.display();

  connectWiFi();
  secured_client.setInsecure();

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(5000);

  taskAll();

  timer.setInterval(SENSOR_UPDATE_MS, taskAll);
  timer.setInterval(RECONNECT_CHECK_MS, ensureConnections);

  if (strlen(CHAT_ID) > 0) {
    bot.sendMessage(String(CHAT_ID), "He thong Nhom 6 da san sang!", "");
  } else {
    Serial.println("CHAT_ID dang de trong. Hay gui /start@ten_bot trong group de lay chat_id tren Serial Monitor.");
  }
}

// ================= LOOP =================
void loop() {
  Blynk.run();
  timer.run();
  checkTelegram();
}