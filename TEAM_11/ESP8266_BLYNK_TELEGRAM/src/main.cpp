/*
  THÔNG TIN NHÓM 11
  1. Nguyễn Đình Hoàng
  2. Nguyễn Đại Quý
  3.Phan Hữu Tuấn Kiệt - Telegram: Kiet dev
  4. Dương Quang Đạt
  5.
  Hệ thống: Điều khiển LED và Giám sát cảm biến qua Telegram / Blynk
*/

#define BLYNK_TEMPLATE_ID "TMPL6AN47FJ2n"
#define BLYNK_TEMPLATE_NAME "BLYNKTELE"
#define BLYNK_AUTH_TOKEN "vbbLKiXsqK96QVN7toifLvJfukFFIvlD"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= TELEGRAM =================
#define BOTtoken "8755277952:AAHLo94cLyqCnOlMFSZ2YFlGYU72ArKy1KE"
#define CHAT_ID "-1003897698538"

// ================= PIN ESP32 =================
#define DHTPIN      15
#define DHTTYPE     DHT22
#define RELAY_PIN   16   // Khớp với LED Cyan trong sơ đồ JSON của bạn
#define MQ2_PIN     32   // Chân Analog Gas
#define OLED_SDA    13
#define OLED_SCL    12

// ================= KHỞI TẠO ĐỐI TƯỢNG =================
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SH1106G display(128, 64, &Wire, -1);
BlynkTimer timer;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);

// ================= BIẾN TOÀN CỤC =================
float temperature = 0;
float humidity = 0;
int gasValue = 0;
bool relayState = false;

unsigned long lastTelegramCheck = 0;
int lastSentGas = -1;

// ================= KẾT NỐI WIFI =================
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK");
}

// ================= ĐỌC CẢM BIẾN =================
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity = h;
  gasValue = analogRead(MQ2_PIN);
}

// ================= CẬP NHẬT OLED =================
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("ESP32 BLYNK TELEGRAM");

  display.setCursor(0, 16);
  display.printf("Temp: %.1f C", temperature);

  display.setCursor(0, 28);
  display.printf("Hum : %.1f %%", humidity);

  display.setCursor(0, 40);
  display.printf("Gas : %d ppm", gasValue);

  display.setCursor(0, 52);
  display.printf("LED : %s", relayState ? "ON" : "OFF");

  display.display();
}

// ================= BLYNK =================
BLYNK_WRITE(V1) { 
  relayState = param.asInt();
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
}

void updateBlynk() {
  Blynk.virtualWrite(V0, millis() / 1000); 
  Blynk.virtualWrite(V2, temperature);    
  Blynk.virtualWrite(V3, humidity);       
  Blynk.virtualWrite(V4, gasValue);       
}

// ================= XỬ LÝ LỆNH TELEGRAM =================
void handleTelegram(int n) {
  for (int i = 0; i < n; i++) {
    String text = bot.messages[i].text;
    String chat_id = String(bot.messages[i].chat_id);
    
    if (text == "/start") {
      String welcome = "Chào mừng Kiệt!\n";
      welcome += "/led_on : Bật đèn\n";
      welcome += "/led_off : Tắt đèn\n";
      welcome += "/led_status : Trạng thái\n";
      welcome += "/get_weather : Cảm biến";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      relayState = true;
      digitalWrite(RELAY_PIN, HIGH);
      Blynk.virtualWrite(V1, 1); 
      bot.sendMessage(chat_id, "✅ Đã bật đèn LED", "");
    }
    else if (text == "/led_off") {
      relayState = false;
      digitalWrite(RELAY_PIN, LOW);
      Blynk.virtualWrite(V1, 0); 
      bot.sendMessage(chat_id, "❌ Đã tắt đèn LED", "");
    }
    else if (text == "/led_status") {
      bot.sendMessage(chat_id, relayState ? "💡 Đèn đang ON" : "🌑 Đèn đang OFF", "");
    }
    else if (text == "/get_weather") {
      String weather = "🌡 THÔNG TIN CẢM BIẾN:\n";
      weather += "Nhiệt độ: " + String(temperature, 1) + "C\n";
      weather += "Độ ẩm: " + String(humidity, 1) + "%\n";
      weather += "Khí Gas: " + String(gasValue) + " ppm";
      bot.sendMessage(chat_id, weather, "");
    }
  }
}

void checkTelegram() {
  if (millis() - lastTelegramCheck > 2000) {
    int n = bot.getUpdates(bot.last_message_received + 1);
    if (n > 0) handleTelegram(n);
    lastTelegramCheck = millis();
  }
}

// ================= CẢNH BÁO GAS =================
void notifyAlert() {
  if (gasValue > 700 && abs(gasValue - lastSentGas) > 100) { 
    bot.sendMessage(CHAT_ID, "⚠️ CẢNH BÁO: Phát hiện rò rỉ Gas (" + String(gasValue) + ")", "");
    lastSentGas = gasValue;
  }
}

void taskAll() {
  readSensors();
  updateOLED();
  updateBlynk();
  notifyAlert();
}

// ================= KHỞI TẠO =================
void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  dht.begin();
  Wire.begin(OLED_SDA, OLED_SCL);
  
  if(!display.begin(0x3C, true)) {
    Serial.println("Lỗi OLED!");
  }

  connectWiFi();
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  secured_client.setInsecure(); 
  
  timer.setInterval(2000, taskAll); 
  bot.sendMessage(CHAT_ID, "Hệ thống Nhóm 11 đã sẵn sàng!", "");
}

void loop() {
  Blynk.run();
  timer.run();
  checkTelegram();
}