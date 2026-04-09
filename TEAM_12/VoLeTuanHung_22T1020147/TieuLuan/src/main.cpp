#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid     = "Wokwi-GUEST";
const char* password = "";

#define BOTtoken  "8641503233:AAGvoo4UG8SeVbysQTKXP1OEAwKbzhJQFxE"
#define CHAT_ID   "5947726417"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

Adafruit_MPU6050 mpu;

int bot_delay = 2000;
unsigned long lastTimeBotRan = 0;
unsigned long lastSendTime   = 0;

bool sendData    = true;
bool ledState    = LOW;
const int ledPin = 2;

// ─── Xử lý lệnh từ Telegram ────────────────
void handleNewMessages(int numNewMessages) {
  Serial.println("Handling New Messages");

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text      = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    Serial.println("CMD: " + text);

    if (text == "/start") {
      String welcome = "Welcome " + from_name + "!\n";
      welcome += "Commands:\n";
      welcome += "/status  - xem gia tri rung\n";
      welcome += "/on      - bat gui du lieu\n";
      welcome += "/off     - tat gui du lieu\n";
      welcome += "/led_on  - bat LED\n";
      welcome += "/led_off - tat LED\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/status") {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      float v = sqrt(
        a.acceleration.x * a.acceleration.x +
        a.acceleration.y * a.acceleration.y +
        a.acceleration.z * a.acceleration.z
      );
      String msg = "Gia tri rung hien tai:\n";
      msg += "Vibration: " + String(v, 2) + " m/s2\n";
      msg += "Nhiet do:  " + String(temp.temperature, 1) + " C\n";
      msg += "Gui du lieu: " + String(sendData ? "BAT" : "TAT");
      bot.sendMessage(chat_id, msg, "");
    }

    if (text == "/on") {
      sendData = true;
      bot.sendMessage(chat_id, "Da BAT gui du lieu", "");
    }

    if (text == "/off") {
      sendData = false;
      bot.sendMessage(chat_id, "Da TAT gui du lieu", "");
    }

    if (text == "/led_on") {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED ON", "");
    }

    if (text == "/led_off") {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED OFF", "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // ← Key fix: dùng CA cert thay vì setInsecure()
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  Wire.begin(21, 22);
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) delay(100);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU6050 OK");

  bot.sendMessage(CHAT_ID, "ESP32 MPU6050 System Started", "");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float vibration = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  Serial.printf("Vibration: %.2f\n", vibration);

  // Cảnh báo rung mạnh
  if (vibration > 15.0) {
    String msg = "CANH BAO RUNG MANH!\nGia tri: " + String(vibration, 2) + " m/s2";
    bot.sendMessage(CHAT_ID, msg, "");
    delay(5000);
  }

  // Báo cáo định kỳ mỗi 5 giây
  if (sendData && millis() - lastSendTime > 5000) {
    String msg = "Bao cao rung:\n";
    msg += "Vibration: " + String(vibration, 2) + " m/s2\n";
    msg += "Nhiet do: " + String(temp.temperature, 1) + " C";
    bot.sendMessage(CHAT_ID, msg, "");
    lastSendTime = millis();
  }

  // Kiểm tra lệnh Telegram mỗi 2 giây
  if (millis() - lastTimeBotRan > bot_delay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("Got Response!");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  delay(500);
}