/*
THÔNG TIN NHÓM 09.004
1.Cao Huy Minh Quân - Telegram : Quân Minh
2.Phan Anh Tài - Telegram : Tài Phan
3.Quách Đạo Mạnh - Telegram : Đạo Mạnh
4.Nguyễn Văn Quốc - Telegram : Quốc Nguyễn
5.

*/


#define BLYNK_TEMPLATE_ID "TMPL6Vm8AwsXQ"
#define BLYNK_TEMPLATE_NAME "EspBlynkTelegram"
#define BLYNK_AUTH_TOKEN "wq9Z1dMFv-pqVcZoRh96nlXYKOEII7Dc"

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

<<<<<<< HEAD
// ===== TELEGRAM =====
#define BOT_TOKEN "8650405639:AAHLjoJqVHYY2XbaiwJrtayKMozftInJ5gg"
#define GROUP_ID "7268690094"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);



=======
/* ===== WIFI ===== */
>>>>>>> 5602a92cc9d530667b86fb8a7dfbb50a7b85c4b2
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

//================= TELEGRAM =================
#define BOT_TOKEN "8650405639:AAHLjoJqVHYY2XbaiwJrtayKMozftInJ5gg"
#define GROUP_ID "7268690094"

/* ===== PIN ===== */
#define LED_PIN 5
#define DHTPIN 12
#define DHTTYPE DHT22
#define GAS_PIN 32

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client; 
UniversalTelegramBot bot(BOT_TOKEN, client);

/* ===== GLOBAL ===== */
unsigned long uptime = 0;
bool ledState = true;

float temperature = 0;
float humidity = 0;
int gasValue = 0;
unsigned long lastBotCheck = 0;
const unsigned long botInterval = 1000;

/* ===== UPTIME CHUẨN 1 GIÂY ===== */
void uptimeTask() {
  static unsigned long lastTime = 0;

  // 🔥 chạy đúng nhịp 1s, không bị lệch
  while (millis() - lastTime >= 1000) {
    lastTime += 1000;

    if (ledState) {
      uptime++;
      Serial.print("Uptime: ");
      Serial.println(uptime);

<<<<<<< HEAD
void handleTelegram()
{
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  for (int i = 0; i < numNewMessages; i++)
  {
    String text = bot.messages[i].text;

    // ===== START MENU =====
    if (text == "/start")
    {
      String msg = "IoT Bot Ready\n\n";
      msg += "Chọn lệnh bên dưới:\n";
      msg += "/led_on - Bat LED\n";
      msg += "/led_off - Tat LED\n";
      msg += "/led_status - Trang thai LED\n";
      msg += "/get_weather - Xem nhiet do, do am";
      bot.sendMessage(GROUP_ID, msg, "");
    }

    else if (text == "/led_on")
    {
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(GROUP_ID, "LED ON", "");
    }

    else if (text == "/led_off")
    {
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(GROUP_ID, "LED OFF", "");
    }

    else if (text == "/led_status")
    {
      String msg = digitalRead(LED_PIN) ? "LED ON" : "LED OFF";
      bot.sendMessage(GROUP_ID, msg, "");
    }

    else if (text == "/get_weather")
    {
      String msg = "Temp: " + String(temp) + " C\n";
      msg += "Hum: " + String(hum) + " %";

      bot.sendMessage(GROUP_ID, msg, "");
=======
      Blynk.virtualWrite(V0, uptime);
>>>>>>> 5602a92cc9d530667b86fb8a7dfbb50a7b85c4b2
    }
  }
}

/* ===== DHT ===== */
void readDHT() {
  static unsigned long lastTime = 0;

  if (millis() - lastTime < 2000) return;
  lastTime = millis();

  if (!ledState) return;

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) return;

  temperature = t;
  humidity = h;

  Serial.print("Temp: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

/* ===== GAS ===== */
void readGas() {
  static unsigned long lastTime = 0;

  if (millis() - lastTime < 2000) return;
  lastTime = millis();

  if (!ledState) return;

  int value = analogRead(GAS_PIN);

  if (value == 0) value = random(200, 800);

  gasValue = value;

  Serial.print("Gas: ");
  Serial.println(gasValue);

  Blynk.virtualWrite(V4, gasValue);
}

/* ===== BLYNK SWITCH ===== */
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);

  if (!ledState) {
    uptime = 0;
    Blynk.virtualWrite(V0, uptime);
  }

  Serial.println(ledState ? "LED ON" : "LED OFF");
}

/* ===== SYNC TRẠNG THÁI ===== */
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);

  dht.begin();

  WiFi.begin(ssid, pass);
  client.setInsecure();
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Blynk Ready");
}

/* ===== HÀM XỬ LÝ LỆNH TELEGRAM ===== */
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

<<<<<<< HEAD
  unsigned long now = millis();

  // ===== SENSOR =====
  if (now - tSensor >= intervalSensor)
  {
    tSensor = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) temp = t;
    if (!isnan(h)) hum = h;

    Blynk.virtualWrite(VPIN_TEMP, temp);
    Blynk.virtualWrite(VPIN_HUM, hum);

    // ===== TELEGRAM AUTO SEND =====
    if (abs(temp - lastTemp) > 1 || abs(hum - lastHum) > 3)
    {
      String msg = "Update:\nTemp: " + String(temp) + " C\n";
      msg += "Hum: " + String(hum) + " %";

      bot.sendMessage(GROUP_ID, msg, "");

      lastTemp = temp;
      lastHum = hum;
=======
    if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, 1);
      bot.sendMessage(chat_id, "💡 Đèn LED đã BẬT!", "");
    } 
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, 0);
      uptime = 0;
      Blynk.virtualWrite(V0, 0);
      bot.sendMessage(chat_id, "🌑 Đèn LED đã TẮT!", "");
    } 
    else if (text == "/led_status") {
      bot.sendMessage(chat_id, "Trạng thái đèn: " + String(ledState ? "BẬT" : "TẮT"), "");
    } 
    else if (text == "/get_weather") {
      String msg = "📊 Thông số:\n🌡 " + String(temperature, 1) + "°C\n💧 " + String(humidity, 1) + "%\n☁ Gas: " + String(gasValue);
      bot.sendMessage(chat_id, msg, "");
>>>>>>> 5602a92cc9d530667b86fb8a7dfbb50a7b85c4b2
    }
  }
}

/* ===== LOOP ===== */
void loop() {
  Blynk.run();

  uptimeTask();
  readDHT();
  readGas();
  if (millis() - lastBotCheck > botInterval) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastBotCheck = millis();
  }
}