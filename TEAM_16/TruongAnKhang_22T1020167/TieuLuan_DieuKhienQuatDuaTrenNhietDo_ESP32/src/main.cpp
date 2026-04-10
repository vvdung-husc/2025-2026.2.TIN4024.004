#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "DHTesp.h"

#define DHT_PIN 15
#define RELAY_PIN 18
#define LED_PIN 19

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ===== TELEGRAM =====
#define BOT_TOKEN "8470254936:AAF3hp1m5lv4iejJsZNSkUbBPNNxEeSvMts"
#define CHAT_ID "8232631690"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

DHTesp dhtSensor;

unsigned long bot_lasttime = 0;
bool fanState = false;
bool alertSent = false;
String mode = "AUTO";   // AUTO hoặc MANUAL

// ===== HÀM BẬT QUẠT =====
void turnFanOn() {
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  fanState = true;
}

// ===== HÀM TẮT QUẠT =====
void turnFanOff() {
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  fanState = false;
}

// ===== GỬI MENU =====
void sendMenu(String chat_id) {
  String menu = "=== DIEU KHIEN QUAT IoT ===\n";
  menu += "Che do hien tai: " + mode + "\n\n";
  menu += "/auto - Chuyen sang che do TU DONG\n";
  menu += "/manual - Chuyen sang che do THU CONG\n";
  menu += "/fan_on - Bat quat\n";
  menu += "/fan_off - Tat quat\n";
  menu += "/temp - Xem nhiet do\n";
  menu += "/status - Xem trang thai\n";
  menu += "/mode - Xem che do hien tai\n";
  menu += "/menu - Hien thi menu";
  bot.sendMessage(chat_id, menu, "");
}

// ===== XỬ LÝ TIN NHẮN =====
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    Serial.println("Tin nhan moi: " + text);

    if (text == "/start" || text == "/menu") {
      sendMenu(chat_id);
    }

    else if (text == "/auto") {
      mode = "AUTO";
      bot.sendMessage(chat_id, "Da chuyen sang che do TU DONG.", "");
    }

    else if (text == "/manual") {
      mode = "MANUAL";
      bot.sendMessage(chat_id, "Da chuyen sang che do THU CONG.", "");
    }

    else if (text == "/fan_on") {
      if (mode == "MANUAL") {
        turnFanOn();
        bot.sendMessage(chat_id, "Da BAT quat (thu cong).", "");
      } else {
        bot.sendMessage(chat_id, "Dang o che do AUTO. Hay gui /manual de dieu khien tay.", "");
      }
    }

    else if (text == "/fan_off") {
      if (mode == "MANUAL") {
        turnFanOff();
        bot.sendMessage(chat_id, "Da TAT quat (thu cong).", "");
      } else {
        bot.sendMessage(chat_id, "Dang o che do AUTO. Hay gui /manual de dieu khien tay.", "");
      }
    }

    else if (text == "/temp") {
      TempAndHumidity data = dhtSensor.getTempAndHumidity();
      String msg = "Nhiet do: " + String(data.temperature) + " °C\n";
      msg += "Do am: " + String(data.humidity) + " %";
      bot.sendMessage(chat_id, msg, "");
    }

    else if (text == "/status") {
      String msg = "Trang thai quat: ";
      msg += (fanState ? "DANG BAT" : "DANG TAT");
      msg += "\nChe do: " + mode;
      bot.sendMessage(chat_id, msg, "");
    }

    else if (text == "/mode") {
      bot.sendMessage(chat_id, "Che do hien tai: " + mode, "");
    }

    else {
      bot.sendMessage(chat_id, "Lenh khong hop le. Gui /menu de xem chuc nang.", "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  turnFanOff();

  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nDa ket noi WiFi");

  client.setInsecure();

  bot.sendMessage(CHAT_ID, "ESP32 da ket noi Telegram Bot!", "");
  bot.sendMessage(CHAT_ID, "Gui /start de hien thi menu dieu khien.", "");
}

void loop() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  float temp = data.temperature;

  // ===== CHẾ ĐỘ AUTO =====
  if (mode == "AUTO") {
    if (temp >= 30) {
      turnFanOn();
    } else {
      turnFanOff();
    }
  }

  // ===== CẢNH BÁO NHIỆT ĐỘ CAO =====
  if (temp >= 35 && !alertSent) {
    bot.sendMessage(CHAT_ID, "CANH BAO: Nhiet do cao! Quat da duoc BAT.", "");
    alertSent = true;
  }

  if (temp < 35) {
    alertSent = false;
  }

  // ===== KIỂM TRA TIN NHẮN TELEGRAM =====
  if (millis() - bot_lasttime > 2000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

  delay(1000);
}