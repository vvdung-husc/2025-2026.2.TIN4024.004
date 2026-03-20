/*
  THÔNG TIN NHÓM 16
  1. Châu Kỳ
  2. Nguyễn Ái Danh
  3. Châu Văn Trường Huy
  4. 
  5. 
*/

#define BLYNK_TEMPLATE_ID "TMPL6GePC9FAO"
#define BLYNK_TEMPLATE_NAME "ESP32BlynkTelegram"
#define BLYNK_AUTH_TOKEN "4bvaSB5fMkGKM_mal8czWRzHhLUXID3T"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR    0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= TELEGRAM =================
#define BOTtoken "8645992932:AAGNLi7s_jUyHqNFLO4ht-KFSOosxhpHq_4"
#define CHAT_ID "-5193448236"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ================= DHT =================
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ================= MQ2 =================
#define GAS_PIN 32

// ================= LED =================
#define LED_PIN 5

// ================= BUZZER =================
#define BUZZER_PIN 15

BlynkTimer timer;

// ================= BIẾN =================
float temp = 0;
float hum = 0;
float lastTemp = 0;
float lastHum  = 0;

int gasValue = 0;
float gasPPM = 0;
float lastGasPPM = 0;

bool gasAlertSent = false;

unsigned long startTime;

// Ngưỡng gas (ppm)
const int GAS_THRESHOLD_HIGH = 800;
const int GAS_THRESHOLD_LOW  = 400;

// ================= ĐỌC CẢM BIẾN =================
void readSensor() {

  // ===== DHT =====
  float newTemp = dht.readTemperature();
  float newHum  = dht.readHumidity();

  if (!isnan(newTemp) && !isnan(newHum)) {
    temp = newTemp;
    hum = newHum;
  }

  // ===== MQ2 =====
  gasValue = analogRead(GAS_PIN);

  // Convert ADC → ppm
  gasPPM = (gasValue / 4095.0) * 1000;

  // ===== BLYNK =====
  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, hum);
  Blynk.virtualWrite(V3, gasPPM);
  Blynk.virtualWrite(V4, (millis() - startTime) / 1000UL);
  Blynk.virtualWrite(V5, "TEAM 16 ");
  // ===== TELEGRAM (TEMP + HUM) =====
  if (abs(temp - lastTemp) > 0.5 || abs(hum - lastHum) > 1) {
    String msg = "Dữ liệu mới từ cảm biến:\n";
    msg += "🌡 Nhiệt độ: " + String(temp, 1) + " C\n";
    msg += "💧 Độ ẩm: " + String(hum, 1) + " %";
    bot.sendMessage(CHAT_ID, msg, "");

    lastTemp = temp;
    lastHum  = hum;
  }

  // ===== CẢNH BÁO GAS (THEO PPM) =====
  if (gasPPM > GAS_THRESHOLD_HIGH) {

    if (!gasAlertSent || abs(gasPPM - lastGasPPM) > 50) {
      String msg = "⚠️ CẢNH BÁO: Phát hiện khí gas!\n Giá trị = " + String(gasPPM, 0);
      bot.sendMessage(CHAT_ID, msg, "");
    }

    digitalWrite(BUZZER_PIN, HIGH);
    gasAlertSent = true;
  }
  else if (gasPPM <= GAS_THRESHOLD_LOW) {
    digitalWrite(BUZZER_PIN, LOW);
    gasAlertSent = false;
  }

  lastGasPPM = gasPPM;

  Serial.printf("T: %.1f°C  H: %.1f%%  Gas: %.0f ppm\n", temp, hum, gasPPM);
}

// ================= OLED =================
void displayOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Nhiệt độ: "); display.print(temp, 1); display.println(" C");
  display.print("Độ ẩm:   ");  display.print(hum, 1);  display.println(" %");
  display.print("Gas:     ");  display.print(gasPPM, 0); display.println(" ppm");

  unsigned long secs = (millis() - startTime) / 1000;
  unsigned long hours = secs / 3600;
  unsigned long mins  = (secs % 3600) / 60;
  unsigned long s     = secs % 60;

  display.print("Uptime:  "); 
  display.printf("%lu:%02lu:%02lu", hours, mins, s);

  display.setCursor(0, 56);
  display.println("Team 16 - IoT");

  display.display();
}

// ================= TELEGRAM =================
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;
      String chat_id = String(bot.messages[i].chat_id);

      if (chat_id != CHAT_ID) continue;

      if (text == "/start") {
        String welcome = "Sử dụng các lệnh sau để điều khiển đèn LED:\n";
        welcome += "Gửi /led_on : để bật đèn\n";
        welcome += "Gửi /led_off : để tắt đèn\n";
        welcome += "Gửi /get_state : để yêu cầu xem trạng thái đèn hiện tại\n";
        welcome += "Gửi /get_weather: để hiển thị nhiệt độ và độ ẩm\n";
        bot.sendMessage(CHAT_ID, welcome, "");
      }
      else if (text == "/led_on") {
        digitalWrite(LED_PIN, HIGH);
        Blynk.virtualWrite(V0, 1);
        bot.sendMessage(CHAT_ID, "LED Đã Bật", "");
      }
      else if (text == "/led_off") {
        digitalWrite(LED_PIN, LOW);
        Blynk.virtualWrite(V0, 0);
        bot.sendMessage(CHAT_ID, "LED Đã Tắt", "");
      }
      else if (text == "/get_state") {
        String status = digitalRead(LED_PIN) ? "Đèn LED đang bật" : "Đèn LED đang tắt";
        bot.sendMessage(CHAT_ID, status, "");
      }
      else if (text == "/get_weather") {
        String msg = "Nhiệt độ: " + String(temp, 1) + " C\n";
        msg += "Độ ẩm: " + String(hum, 1) + " %";
        bot.sendMessage(CHAT_ID, msg, "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ================= BLYNK =================
BLYNK_WRITE(V0) {
  digitalWrite(LED_PIN, param.asInt());
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin(27, 26);

  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED error");
    while(1);
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Khởi động...");
  display.display();

  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  client.setInsecure();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  startTime = millis();

  timer.setInterval(2000L, readSensor);
  timer.setInterval(1000L, handleTelegram);
  timer.setInterval(2000L, displayOLED);
}

// ================= LOOP =================
void loop() {
  Blynk.run();
  timer.run();
}