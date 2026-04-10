#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

const char* ssid     = "Wokwi-GUEST";
const char* password = "";

#define BOTtoken "8641503233:AAGvoo4UG8SeVbysQTKXP1OEAwKbzhJQFxE"
#define CHAT_ID  "5947726417"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

Adafruit_MPU6050 mpu;

unsigned long lastTimeBotRan = 0;
unsigned long lastSendTime   = 0;
unsigned long lastAlertTime  = 0;

// sendData = true  → gửi tất cả lên Telegram (báo cáo + cảnh báo)
// sendData = false → im lặng hoàn toàn trên Telegram, terminal vẫn chạy
bool sendData = true;
bool ledState = false;
const int ledPin       = 2;
const float NGUONG_RUNG = 15.0;

// ════════════════════════════════════════
//  GIẢ LẬP DỮ LIỆU RUNG
// ════════════════════════════════════════
float t = 0.0;

float getRungDong() {
  t += 0.5;
  float ax = 1.5 * sin(t * 0.8) + (random(-20, 20) / 100.0);
  float ay = 1.2 * cos(t * 0.6) + (random(-20, 20) / 100.0);
  float az = 9.81 + 0.4 * sin(t * 1.1) + (random(-15, 15) / 100.0);

  float rung = sqrt(ax*ax + ay*ay + az*az);
  
  if (random(0, 100) < 30) rung += random(6, 10);
  return rung;
}

float getNhietDo() {
  return 24.0 + 0.8 * sin(t * 0.05) + (random(-3, 3) / 10.0);
}

// ─── Bật/tắt LED ───────────────────────
void setLED(bool on) {
  if (ledState == on) return;
  ledState = on;
  digitalWrite(ledPin, on ? HIGH : LOW);
  Serial.println("LED: " + String(on ? "ON (canh bao)" : "OFF (binh thuong)"));
}

// ─── Telegram commands ─────────────────
void handleNewMessages(int n) {
  for (int i = 0; i < n; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) { bot.sendMessage(chat_id, "Unauthorized", ""); continue; }

    String text = bot.messages[i].text;
    String name = bot.messages[i].from_name;

    if (text == "/start") {
      String msg = "Xin chao " + name + "!\n";
      msg += "/status - xem rung dong hien tai\n";
      msg += "/on - bat gui tat ca thong bao\n";
      msg += "/off - tat tat ca thong bao Telegram";
      bot.sendMessage(chat_id, msg, "");
    }

    if (text == "/status") {
      float rd = getRungDong();
      float nd = getNhietDo();
      String msg = "Rung dong: " + String(rd, 2) + " m/s2 | Nhiet do: " + String(nd, 1) + " C";
      msg += " | LED: " + String(ledState ? "ON" : "OFF");
      msg += " | Telegram: " + String(sendData ? "BAT" : "TAT");
      bot.sendMessage(chat_id, msg, "");
    }

    if (text == "/on") {
      sendData = true;
      bot.sendMessage(chat_id, "Da BAT gui thong bao", "");
    }

    if (text == "/off") {
      sendData = false;
      bot.sendMessage(chat_id, "Da TAT gui thong bao", "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" OK");

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  Wire.begin(21, 22);
  if (mpu.begin()) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  bot.sendMessage(CHAT_ID, "ESP32 bat dau!\n/on - bat thong bao\n/off - tat thong bao", "");
}

void loop() {
  float rd = getRungDong();
  float nd = getNhietDo();

  // Terminal luôn hiển thị dù sendData = true hay false
  Serial.println("Rung dong: " + String(rd, 2) + " m/s2 | Nhiet do: " + String(nd, 1) + " C | LED: " + String(ledState ? "ON" : "OFF"));

  // LED vẫn tự động bật/tắt theo rung, bất kể sendData
  if (rd > NGUONG_RUNG) {
    setLED(true);
    // Chỉ gửi Telegram nếu sendData = true
    if (sendData && millis() - lastAlertTime > 5000) {
      String msg = "CANH BAO RUNG MANH!\n";
      msg += "Rung dong: " + String(rd, 2) + " m/s2\n";
      msg += "Nhiet do: " + String(nd, 1) + " C\n";
      msg += "Den LED: On (canh bao)";
      bot.sendMessage(CHAT_ID, msg, "");
      lastAlertTime = millis();
    }
  } else {
    setLED(false);
  }

  // Báo cáo định kỳ mỗi 5 giây — chỉ gửi nếu sendData = true
  if (sendData && millis() - lastSendTime > 5000) {
    String msg = "Rung dong: " + String(rd, 2) + " m/s2 | Nhiet do: " + String(nd, 1) + " C";
    msg += " | Den LED: " + String(ledState ? "ON (canh bao)" : "OFF (binh thuong)");
    bot.sendMessage(CHAT_ID, msg, "");
    lastSendTime = millis();
  }

  // Kiểm tra lệnh Telegram mỗi 2 giây
  if (millis() - lastTimeBotRan > 2000) {
    int n = bot.getUpdates(bot.last_message_received + 1);
    while (n) { handleNewMessages(n); n = bot.getUpdates(bot.last_message_received + 1); }
    lastTimeBotRan = millis();
  }

  delay(500);
}