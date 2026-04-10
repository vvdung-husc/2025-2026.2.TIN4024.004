#define BLYNK_TEMPLATE_ID "TMPL6VptCfODh"
#define BLYNK_TEMPLATE_NAME "TieuLuanIoT"
#define BLYNK_AUTH_TOKEN "SambOADce_Tb4ej1kDUmVXOF9SXW-gpj"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <UniversalTelegramBot.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define BOTtoken "8778048306:AAExPPULrubga1SJdq_gHb5LuOIXc86nfHM"
#define CHAT_ID  "-5165562658"

// ================== PIN ==================
#define PIN_DS18B20     4
#define PIN_LDR         34
#define PIN_PH          36
#define PIN_RELAY       2
#define PIN_LED_PH      25
#define PIN_LED_LIGHT   26
#define PIN_LED_PUMP    27
#define PIN_DHT         16
#define DHTTYPE DHT22

// ================== BLYNK VIRTUAL PINS ==================
// V0 — Gauge  : Nhiệt độ nước (0–50 °C)
// V1 — Gauge  : pH            (0–14)
// V2 — Gauge  : Ánh sáng      (0–100 %)
// V3 — LED    : Bơm           (0 / 255)
// V4 — Button : Bật/tắt bơm thủ công  (SWITCH)
// V5 — Button : Chế độ AUTO/MANUAL    (SWITCH)
// V6 — LED    : Đèn trồng     (0 / 255)
// V7 — LED    : Quạt/nhiệt    (0 / 255)

// ================== BIẾN ==================
bool manualMode = false;
bool manualPump = false;

bool errorPH = false, errorTemp = false, errorLight = false;
bool errorTempHI = false, errorTempLO = false;

bool lastErrorPH = false, lastErrorTemp = false, lastErrorLight = false;
bool lastErrorTempHI = false, lastErrorTempLO = false;

float tempWater = 0, tempAir = 0, humidity = 0, pH = 7;
int lightPercent = 0;


unsigned long lastTelegramCheck = 0;
#define TELEGRAM_INTERVAL 1000

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);
DHT dht(PIN_DHT, DHTTYPE);

BlynkTimer timer;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ================== UPTIME ==================
String getUptime() {
  unsigned long s = millis() / 1000;
  return String(s / 3600) + "h " + String((s % 3600) / 60) + "m";
}

// ================== TELEGRAM HELPERS ==================

// Gửi cảnh báo tự động tới group (không kèm nút)
void sendTelegram(String msg) {
  bot.sendMessage(CHAT_ID, msg, "");
}

// Gửi tin kèm nút 🏠 Menu
void sendTelegramWithMenu(String chat_id, String msg) {
  String keyboard = "[[{\"text\":\"🏠 Menu\",\"callback_data\":\"/start\"}]]";
  bot.sendMessageWithInlineKeyboard(chat_id, msg, "", keyboard);
}

// Gửi menu chính
void sendMainMenu(String chat_id) {
  String msg = "🌿 *HỆ THỐNG THỦY CANH*\n";
  msg += "──────────────────\n";
  msg += "Chế độ : " + String(manualMode ? "🎮 MANUAL" : "🤖 AUTO") + "\n";
  msg += "──────────────────\n";
  msg += "Chọn lệnh bên dưới:";

  String keyboard = "[[";
  keyboard += "{\"text\":\"🤖 AUTO\",\"callback_data\":\"/auto_mode\"},";
  keyboard += "{\"text\":\"🎮 MANUAL\",\"callback_data\":\"/manual_mode\"}";
  keyboard += "],[";
  keyboard += "{\"text\":\"📊 Xem thông số\",\"callback_data\":\"/get_state\"}";
  keyboard += "]]";

  bot.sendMessageWithInlineKeyboard(chat_id, msg, "Markdown", keyboard);
}

// Gửi thông số hiện tại
void sendState(String chat_id) {
  String msg = "📊 *THÔNG SỐ HIỆN TẠI*\n";
  msg += "──────────────────\n";
  msg += "🌡 Nhiệt độ nước : " + String(tempWater, 1) + " °C" + (errorTemp  ? " ⚠️" : " ✅") + "\n";
  msg += "🌡 Nhiệt độ KK   : " + String(tempAir,   1) + " °C\n";
  msg += "💧 Độ ẩm         : " + String(humidity,  0) + " %\n";
  msg += "🧪 pH            : " + String(pH,         2) + (errorPH    ? " ⚠️" : " ✅") + "\n";
  msg += "☀️ Ánh sáng      : " + String(lightPercent)  + " %" + (errorLight ? " ⚠️" : " ✅") + "\n";
  msg += "──────────────────\n";
  msg += "💡 Đèn  : " + String(errorLight ? "ON" : "OFF") + "\n";
  msg += "🌀 Quạt : " + String(errorTempHI  ? "ON" : "OFF") + "\n";
  msg += "💧 Bơm : " + String(errorPH ? "ON" : "OFF") + "\n"; 
  sendTelegramWithMenu(chat_id, msg);
}

// ================== TELEGRAM HANDLER ==================
void handleTelegram() {
  int n = bot.getUpdates(bot.last_message_received + 1);
  while (n) {
    for (int i = 0; i < n; i++) {
      String chat_id = bot.messages[i].chat_id;
      String type    = bot.messages[i].type;
      String text    = bot.messages[i].text;

      Serial.println("TG type: " + type + " | text: " + text);

      if (text == "/start") {
        sendMainMenu(chat_id);
      }
      else if (text == "/get_state") {
        sendState(chat_id);
      }
      else if (text == "/auto_mode") {
        manualMode = false;
        Blynk.virtualWrite(V5, 0);
        sendTelegramWithMenu(chat_id, "🤖 Đã chuyển sang AUTO");
      }
      else if (text == "/manual_mode") {
        manualMode = true;
        Blynk.virtualWrite(V5, 1);
        sendTelegramWithMenu(chat_id, "🎮 Đã chuyển sang MANUAL");
      }

      // Xoá loading spinner trên nút inline
      if (type == "callback_query") {
        bot.answerCallbackQuery(bot.messages[i].query_id, "✅");
      }
    }
    n = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ================== BLYNK WRITE ==================

// V4 — Nút bơm thủ công (Button SWITCH)
BLYNK_WRITE(V4) {
  int val = param.asInt();

  Serial.print("V4 value: ");
  Serial.println(val);  // DEBUG

  if (manualMode) {
    if (val == 1) {
      manualPump = true;
      sendTelegram("🟢 [Blynk] Bơm/Quạt gió/Đèn trồng BẬT (Manual)");
    } else {
      manualPump = false;
      sendTelegram("🔴 [Blynk] Bơm/Quạt gió/Đèn trồng TẮT (Manual)");
    }
  } 
  else {
    Blynk.virtualWrite(V4, 0);
    sendTelegram("⚠️ Đang ở AUTO - Không thể điều khiển");
  }
}

// V5 — Nút chế độ (Button SWITCH)
BLYNK_WRITE(V5) {
  int val = param.asInt();
  manualMode = (val == 1);
  sendTelegram(manualMode ? "🎮 [Blynk] Chuyển MANUAL" : "🤖 [Blynk] Chuyển AUTO");
}

// ================== UPDATE SYSTEM ==================
void updateSystem() {

  // ===== ĐỌC SENSOR =====
  sensors.requestTemperatures();
  tempWater = sensors.getTempCByIndex(0);

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) tempAir = t;
  if (!isnan(h)) humidity = h;

  // ===== ĐỌC ANALOG =====
  int rawPH = 0, rawLDR = 0;
  for (int i = 0; i < 5; i++) {
    rawPH  += analogRead(PIN_PH);
    rawLDR += analogRead(PIN_LDR);
    delay(5);
  }
  rawPH  /= 5;
  rawLDR /= 5;

  pH           = map(rawPH,  0, 4095, 0,   1400) / 100.0;
  lightPercent = map(rawLDR, 0, 4095, 100, 0);

  // ===== LOGIC =====
  errorPH    = (pH < 5.5 || pH > 6.5);
  errorTempHI = (tempWater > 26);     // NÓNG → Quạt V7 ON  
  errorTempLO = (tempWater < 18); 
  errorLight = (lightPercent < 40);
  //bool pumpOn = manualMode ? manualPump : (errorPH || errorTempHI);  
  //bool pumpOn = manualMode ? manualPump : (errorPH || errorTemp);
  bool pumpOn = manualMode ? manualPump : errorPH;
  // ===== OUTPUT =====
  digitalWrite(PIN_RELAY,     pumpOn     ? LOW  : HIGH);
  digitalWrite(PIN_LED_PH,    errorPH    ? HIGH : LOW);
  digitalWrite(PIN_LED_LIGHT, errorLight ? HIGH : LOW);
  digitalWrite(PIN_LED_PUMP,  errorTempHI  ? HIGH : LOW);

  // ===== OLED =====
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.printf("UP: %s\n",        getUptime().c_str());
  display.printf("Water:%.1f%s\n",  tempWater,    errorTemp  ? "!!" : "  ");
  display.printf("pH:%.2f%s\n",     pH,           errorPH    ? "!!" : "  ");
  display.printf("Light:%d%%%s\n",  lightPercent, errorLight ? "LOW" : "   ");
  display.printf("Air:%.1fC\n",     tempAir);
  display.printf("Hum:%.0f%%\n",    humidity);
  display.printf("Pump:%s %s\n",    pumpOn ? "ON" : "OFF", manualMode ? "(M)" : "(A)");
  display.display();

  // ===== BLYNK =====
  Blynk.virtualWrite(V0, tempWater);
  Blynk.virtualWrite(V1, pH);
  Blynk.virtualWrite(V2, lightPercent);
  Blynk.virtualWrite(V3, pumpOn     ? 255 : 0);
  Blynk.virtualWrite(V6, errorLight ? 255 : 0);
  Blynk.virtualWrite(V7, errorTempHI  ? 255 : 0);

  // ===== TELEGRAM CẢNH BÁO =====
  // Chỉ gửi đúng 1 lần khi trạng thái VỪA chuyển sang lỗi
  String msg = "";

  if (errorPH && !lastErrorPH) {
    msg  = "⚠️ CẢNH BÁO pH\n";
    msg += "pH = " + String(pH, 2) + " → bật bơm";
    sendTelegram(msg);
  }
  if (errorTempHI && !lastErrorTempHI) {  // NÓNG
  msg  = "🔥 NHIỆT ĐỘ NÓNG\n";
  msg += String(tempWater,1) + "°C > 26°C → bật quạt";
  sendTelegram(msg);
}
  if (errorTempLO && !lastErrorTempLO) {   // LẠNH  
  msg  = "❄️ NHIỆT ĐỘ LẠNH\n";
  msg += String(tempWater,1) + "°C < 18°C → Cảnh báo nước lạnh";
  sendTelegram(msg);
}
  if (errorLight && !lastErrorLight) {
    msg  = "⚠️ CẢNH BÁO ÁNH SÁNG\n";
    msg += String(lightPercent) + "% < 40% → bật đèn trồng";
    sendTelegram(msg);
  }
  if ((lastErrorPH || lastErrorTemp || lastErrorLight) &&
      !errorPH && !errorTemp && !errorLight) {
    msg  = "✅ MÔI TRƯỜNG ỔN ĐỊNH\n";
    msg += "pH: "    + String(pH, 2)        + " | ";
    msg += "Water: "  + String(tempWater, 1) + "°C | ";
    msg += "Light: " + String(lightPercent) + "%";
    sendTelegram(msg);
  }

  lastErrorPH    = errorPH;
  lastErrorTempHI = errorTempHI;
  lastErrorTempLO = errorTempLO;
  lastErrorLight = errorLight;
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_RELAY,     OUTPUT);
  pinMode(PIN_LED_PH,    OUTPUT);
  pinMode(PIN_LED_LIGHT, OUTPUT);
  pinMode(PIN_LED_PUMP,  OUTPUT);
  digitalWrite(PIN_RELAY, HIGH);

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    Serial.println("OLED FAIL");

  dht.begin();
  sensors.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  client.setInsecure();

  sendTelegram("🚀 Hệ thống khởi động\nGõ /start để mở menu");
  timer.setInterval(3000L, updateSystem);
}

// ================== LOOP ==================
void loop() {
  Blynk.run();
  timer.run();

  // Telegram kiểm tra mỗi 1 giây, không tranh WiFi với Blynk
  if (millis() - lastTelegramCheck > TELEGRAM_INTERVAL) {
    lastTelegramCheck = millis();
    handleTelegram();
  }
}