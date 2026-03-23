/*
  Nhom 6
  1. Nguyen Huu Trinh - 22T1020481
  2. Ho Sy Vinh - 22T1020521
  3. Nguyễn Đăng Bảo Toàn -22T1020470



*/


#define BLYNK_TEMPLATE_ID "TMPL6QeLqro-4"
#define BLYNK_TEMPLATE_NAME "ESP32 Team 06"
#define BLYNK_AUTH_TOKEN    "xxxx"// ae ai làm được cái blynk thì điền hộ vào đống này với

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define BLYNK_PRINT Serial

// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= TELEGRAM =================
// Hay thay bang token moi sau khi ban regenerate token
const char* BOT_TOKEN = "8442569741:AAFc2pbFqw_xFb4RG9V4u0RHvpNFsrSjJi0";

// De rong o lan chay dau tien de lay chat_id tu Serial Monitor
const char* CHAT_ID = "-1003832029526";

// ================= THONG TIN NHOM =================
const char* GROUP_INFO = "IOT-Team06.004";

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


// ================= WIFI =================


// ================= DOC CAM BIEN =================


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

    
    if (strlen(CHAT_ID) > 0 && chat_id != String(CHAT_ID)) {
      bot.sendMessage(chat_id, "Bot chỉ xử lí lệnh đã cấu hình.", "");
      continue;
    }

    if (command == "/start") {
      String welcome = "Chào mừng đến với nhóm 6!\n";
      welcome += "/led_on - Bật LED\n";
      welcome += "/led_off - Tắt LED\n";
      welcome += "/led_status - Xem trạng thái LED\n";
      welcome += "/get_weather - Xem nhiệt độ hiện tại";
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