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

// ================== WIFI ==================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define BOTtoken "8778048306:AAExPPULrubga1SJdq_gHb5LuOIXc86nfHM"
#define CHAT_ID  "-5165562658"

// ================== PIN ==================
#define PIN_DS18B20     4
#define PIN_LDR         34
#define PIN_PH          36
#define PIN_RELAY       2

#define PIN_LED_PH      25   // LED tím
#define PIN_LED_LIGHT   26   // LED vàng (đèn trồng)
#define PIN_LED_PUMP    27   // LED xanh (bơm)

#define PIN_DHT         16
#define DHTTYPE DHT22

bool manualMode = false;
bool manualPump = false;
// ================== OLED ==================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================== SENSOR ==================
OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);
DHT dht(PIN_DHT, DHTTYPE);

// ================== SYSTEM ==================
BlynkTimer timer;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ================== BIẾN ==================
float tempWater = 0, tempAir = 0, humidity = 0, pH = 7;
int lightPercent = 0;

bool errorPH = false, errorTemp = false, errorLight = false;
bool lastErrorPH = false, lastErrorTemp = false, lastErrorLight = false;

// ================== UPTIME ==================
String getUptime() {
  unsigned long s = millis() / 1000;
  return String(s/3600) + "h " + String((s%3600)/60) + "m";
}

// ================== TELEGRAM ==================
void sendTelegram(String msg) {
  Serial.println("Send TG:");
  Serial.println(msg);
  bot.sendMessage(CHAT_ID, msg, "");
}

void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;
      String chat_id = bot.messages[i].chat_id;

      if (text == "/start") {
        String msg = "📊 TRẠNG THÁI HỆ THỐNG\n";
        msg += "🌡 Nước: " + String(tempWater,1) + "°C\n";
        msg += "🌡 Không khí: " + String(tempAir,1) + "°C\n";
        msg += "💧 Độ ẩm: " + String(humidity,0) + "%\n";
        msg += "🧪 pH: " + String(pH,2) + "\n";
        msg += "☀️ Ánh sáng: " + String(lightPercent) + "%\n";
        msg += "🚿 Bơm: " + String(digitalRead(PIN_RELAY)==LOW ? "ON" : "OFF");

        bot.sendMessage(chat_id, msg, "");
      }

      if (text == "/pump_on") {
        manualMode = true;
        manualPump = true;
        bot.sendMessage(chat_id, "🟢 Bơm đã bật (Manual)", "");
      }

      if (text == "/pump_off") {
        manualMode = true;
        manualPump = false;
        bot.sendMessage(chat_id, "🔴 Bơm đã tắt (Manual)", "");
      }

      if (text == "/auto_mode") {
        manualMode = false;
        bot.sendMessage(chat_id, "🤖 AUTO mode", "");
      }

      if (text == "/manual_mode") {
        manualMode = true;
        bot.sendMessage(chat_id, "🎮 MANUAL mode", "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}
// ================== MAIN ==================
void updateSystem() {

  // ===== ĐỌC SENSOR =====
  sensors.requestTemperatures();
  tempWater = sensors.getTempCByIndex(0);

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) tempAir = t;
  if (!isnan(h)) humidity = h;

  // ===== ĐỌC ANALOG (lọc nhiễu) =====
  int rawPH = 0;
  int rawLDR = 0;

  for (int i = 0; i < 5; i++) {
    rawPH += analogRead(PIN_PH);
    rawLDR += analogRead(PIN_LDR);
    delay(5);
  }

  rawPH /= 5;
  rawLDR /= 5;

  pH = map(rawPH, 0, 4095, 0, 1400) / 100.0;
  lightPercent = map(rawLDR, 0, 4095, 100, 0);

  // ===== LOGIC =====
  errorPH   = (pH < 5.5 || pH > 7.5);
  errorTemp = (tempWater > 30);
  errorLight = (lightPercent < 40);

  //bool pumpOn = errorPH || errorTemp;
  bool pumpOn;

  if (manualMode) {
  pumpOn = manualPump;
  } else {
  pumpOn = errorPH || errorTemp;
  }

  // ===== OUTPUT =====
  digitalWrite(PIN_RELAY, pumpOn ? LOW : HIGH);

  digitalWrite(PIN_LED_PH, errorPH ? HIGH : LOW);
  digitalWrite(PIN_LED_LIGHT, errorLight ? HIGH : LOW);  // LED vàng = đèn trồng
  digitalWrite(PIN_LED_PUMP, errorTemp ? HIGH : LOW);

  // ===== OLED =====
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.printf("UP: %s\n", getUptime().c_str());
  display.printf("Water: %.1f %s\n", tempWater, errorTemp ? "!!" : "");
  display.printf("pH: %.2f %s\n", pH, errorPH ? "!!" : "");
  display.printf("Light: %d%% %s\n", lightPercent, errorLight ? "LOW" : "");
  display.printf("Air: %.1fC\n", tempAir);
  display.printf("Hum: %.0f%%\n", humidity);
  display.printf("Pump: %s", pumpOn ? "ON" : "OFF");

  display.display();

  // ===== BLYNK =====
  Blynk.virtualWrite(V0, tempWater);
  Blynk.virtualWrite(V1, pH);
  Blynk.virtualWrite(V2, lightPercent);
  Blynk.virtualWrite(V3, pumpOn ? 255 : 0);

  String msg = "";

  // ===== pH =====
  if (errorPH && !lastErrorPH) {
    msg = "⚠️ CẢNH BÁO THỦY CANH\n";
    msg += "pH = " + String(pH,2) + "\n";
    msg += "pH vượt ngưỡng → bật bơm";
    sendTelegram(msg);
  }

  // ===== NHIỆT =====
  if (errorTemp && !lastErrorTemp) {
    msg = "⚠️ CẢNH BÁO THỦY CANH\n";
    msg += "Nhiệt độ = " + String(tempWater,1) + "°C\n";
    msg += ">30°C → bật bơm";
    sendTelegram(msg);
  }

  // ===== ÁNH SÁNG =====
  if (errorLight && !lastErrorLight) {
    msg = "⚠️ CẢNH BÁO THỦY CANH\n";
    msg += "Ánh sáng = " + String(lightPercent) + "%\n";
    msg += "<40% → bật đèn trồng";
    sendTelegram(msg);
  }

  // ===== ỔN ĐỊNH =====
  if ((lastErrorPH || lastErrorTemp || lastErrorLight) &&
      !errorPH && !errorTemp && !errorLight) {

    msg = "✅ MÔI TRƯỜNG ỔN ĐỊNH\n";
    msg += "pH: " + String(pH,2) + "\n";
    msg += "Temp: " + String(tempWater,1) + "°C\n";
    msg += "Light: " + String(lightPercent) + "%";
    sendTelegram(msg);
  }

  lastErrorPH = errorPH;
  lastErrorTemp = errorTemp;
  lastErrorLight = errorLight;
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED_PH, OUTPUT);
  pinMode(PIN_LED_LIGHT, OUTPUT);
  pinMode(PIN_LED_PUMP, OUTPUT);

  digitalWrite(PIN_RELAY, HIGH);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAIL");
  }

  dht.begin();
  sensors.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  client.setInsecure();

  sendTelegram("🚀 System started");

  timer.setInterval(3000L, updateSystem);
}

// ================== LOOP ==================
void loop() {
  Blynk.run();
  timer.run();
  handleTelegram(); 
}