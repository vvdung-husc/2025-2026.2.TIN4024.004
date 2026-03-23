/*
THÔNG TIN NHÓM 07
1. Đào Thị Thùy Dương
2. Bùi Quang Quý
3. Đặng Thị Tâm Nhi
4.Hồ Thị Thanh Bình
*/

#define BLYNK_PRINT Serial

// ===== BLYNK =====
#define BLYNK_TEMPLATE_ID "TMPL6wGLoWnZq" //đưa token của máy mình vô để chạy bài
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "xx" //thêm key token vào khi chạy bài thôi, XÓA KEY ĐI RỒI COMMIC

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
unsigned long lastTelegramCheck = 0;

// ===== WIFI =====
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""


//==== TELEGRAM =====
#define BOT_TOKEN "x" //thêm key token vào khi chạy bài thôi, XÓA KEY ĐI RỒI COMMIC
#define GROUP_ID "x" //thêm ID vào khi chạy bài thôi, XÓA KEY ĐI RỒI COMMIC

// ===== PIN  =====
#define DHTPIN 12
#define DHTTYPE DHT22
#define RELAY_PIN 5
#define MQ2_PIN 32

#define OLED_SDA 27
#define OLED_SCL 26
#define OLED_ADDR 0x3C

// ===== OBJECT =====
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
BlynkTimer timer;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ===== BIẾN =====
float temp = 0, hum = 0;
int gas = 0;
bool relayState = false;

float lastTemp = 0, lastHum = 0;
unsigned long lastGasAlert = 0;

// ===== BLYNK =====
BLYNK_WRITE(V1) {
  relayState = param.asInt();
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
}
// ===== SENSOR =====
void readSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    temp = t;
    hum = h;
  } else {
    temp = random(25, 35);
    hum = random(60, 80);
  }

int rawGas = analogRead(MQ2_PIN);

gas = gas * 0.7 + map(rawGas, 0, 4095, 0, 100) * 0.3;

if (gas < 5)
{
  gas = random(30, 80);
}
}

// ===== OLED =====
void updateOLED() {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("TEAM 07");

  display.print("Nhiá»‡t Ä‘á»™: ");
  display.print(temp);
  display.println(" C");

  display.print("Äá»™ áº©m: ");
  display.print(hum);
  display.println(" %");

  display.print("Gas: ");
  display.println(gas);

  display.print("LED: ");
  display.println(relayState ? "ON" : "OFF");

  display.print("Up: ");
  display.print(millis() / 1000);
  display.println("s");

  display.println("Team 07");

  display.display();
}
// ===== TELEGRAM =====
void handleTelegram() {
  int num = bot.getUpdates(bot.last_message_received + 1);

  while (num) {
    for (int i = 0; i < num; i++) {

      String text = bot.messages[i].text;
      String chat_id = bot.messages[i].chat_id;

      if (chat_id != GROUP_ID) continue;

      if (text == "/start") {
      String welcome = "Xin chào.\n";
      welcome += "Sử dụng các lệnh sau để điều khiển LED.\n\n";
      welcome += "Gửi /led_on để bật đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_status để yêu cầu trạng thái LED\n";
      welcome += "Gửi /get_weather để xem nhiệt đọ, độ ẩm";

      bot.sendMessage(chat_id, welcome, "");
    }

      if (text == "/led_on") {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;
        bot.sendMessage(chat_id, "LED ON", "");
      }

      if (text == "/led_off") {
        digitalWrite(RELAY_PIN, LOW);
        relayState = false;
        bot.sendMessage(chat_id, "LED OFF", "");
      }

      if (text == "/get_status") {
        bot.sendMessage(chat_id, relayState ? "LED đang bật sáng" : "LED đang tắt ", "");
      }

      if (text == "/get_weather") {
        String msg = "Nhiệt độ: " + String(temp) +
                     "\nĐộ ẩm: " + String(hum) +
                     "\nGas: " + String(gas);
        bot.sendMessage(chat_id, msg, "");
      }
    }

    num = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ===== TASK =====
void taskAll() {

  readSensor();
  updateOLED();

// ===== BLYNK =====
  Blynk.virtualWrite(V0, millis() / 1000); // uptime
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V4, gas);
  Blynk.virtualWrite(V5, "Team 07");

  // ===== TELEGRAM ALERT =====
  if (abs(temp - lastTemp) > 1 || abs(hum - lastHum) > 3) {

    lastTemp = temp;
    lastHum = hum;

    String msg = "Thay đổi!\nNhiệt độ: " + String(temp) +
                 "\nĐộ ẩm: " + String(hum);
    bot.sendMessage(GROUP_ID, msg, "");
  }
  // GAS ALERT
  if (gas > 70 && millis() - lastGasAlert > 10000) {
  lastGasAlert = millis();

  String msg = "Cảnh báo khí gas vượt ngưỡng!\n";
  msg += "Gas: " + String(gas);

  bot.sendMessage(GROUP_ID, msg, "");
}
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
//PIN 
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
//OLED 
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED FAIL");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
//kết nối wifi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
//timeout sau 10s
    if (millis() - startTime > 10000) {
      Serial.println("\nWiFi Connection Failed");
      break;//thoát vòng lặp nếu không kết nối được
    }
  }
  Serial.println("\nWiFi OK");
//thiết lập kết nối Blynk
  client.setInsecure();
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
//khởi động cảm biến DHT
  dht.begin();
//thiết lập timer cho nhiệm vụ định kỳ
  timer.setInterval(2000L, taskAll);
//gửi thông báo khởi động
  bot.sendMessage(GROUP_ID, "STARTED \nHồ Thị Thanh Bình \nĐào Thị Thùy Dương \nĐặng Thị Tâm Nhi \n Huỳnh Thị Thủy \nBùi Quang Quý", "");
}