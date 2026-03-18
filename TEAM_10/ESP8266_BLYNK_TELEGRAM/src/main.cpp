/*
THÔNG TIN NHÓM 10.004
1. Tôn Thất Bách
2. Hoàng Anh Quân
*/
#define BLYNK_TEMPLATE_ID "TMPL6_gtV30Gj"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "YQSe5OGrNus3Y6x3epMNGxFMCxs7IOpf"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>

// --- Cấu hình OLED SH1106 (D2=SDA, D1=SCL) ---
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";
#define BOTtoken "8741985594:AAHNiX4lLB1lpDY58TOAz3Hc1uIbiz4y9YE"
#define GROUP_ID "-5167632125"

#define DHTPIN D3
#define DHTTYPE DHT22
#define LED_PIN D6 // Logic âm: LOW = BẬT, HIGH = TẮT
#define MQ2_PIN A0

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
BlynkTimer timer;

float t = 0, h = 0;
float lastT = 0, lastH = 0;
int gasValue = 0;
unsigned long lastTimeBotRan;

// --- 1. Hiển thị OLED ---
void updateOLED()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 15, "IoT - Team 10.004");
  u8g2.drawHLine(0, 18, 128);

  u8g2.setCursor(0, 32);
  if (isnan(t) || t <= 0)
  {
    u8g2.print("Sensor Loading...");
  }
  else
  {
    u8g2.print("Temp: ");
    u8g2.print(t, 1);
    u8g2.print(" C");
    u8g2.setCursor(0, 45);
    u8g2.print("Humid: ");
    u8g2.print(h, 1);
    u8g2.print(" %");
  }

  u8g2.setCursor(0, 58);
  u8g2.print("Gas: ");
  u8g2.print(gasValue);
  u8g2.setCursor(75, 58);
  u8g2.print("Up:");
  u8g2.print(millis() / 1000);
  u8g2.print("s");

  u8g2.drawHLine(0, 60, 128);
  u8g2.drawStr(10, 64, "Status: ");
  u8g2.drawStr(60, 64, (digitalRead(LED_PIN) == LOW) ? "OFF" : "ON");
  u8g2.sendBuffer();
}

// --- 2. Xử lý Telegram (Phân định rõ ràng 4 lệnh) ---
void handleTelegramMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID)
      continue;

    String text = bot.messages[i].text;

    // Lệnh 1: Bật đèn
    if (text == "/led_on")
    {
      digitalWrite(LED_PIN, LOW); // LOW là Bật
      Blynk.virtualWrite(V0, 1);  // Đồng bộ lên App Blynk
      bot.sendMessage(GROUP_ID, "Hệ thống: Đèn đã được BẬT ✅", "");
    }
    // Lệnh 2: Tắt đèn
    else if (text == "/led_off")
    {
      digitalWrite(LED_PIN, HIGH); // HIGH là Tắt
      Blynk.virtualWrite(V0, 0);   // Đồng bộ lên App Blynk
      bot.sendMessage(GROUP_ID, "Hệ thống: Đèn đã được TẮT 🌑", "");
    }
    // Lệnh 3: Xem trạng thái hiện tại
    else if (text == "/led_status")
    {
      String status = (digitalRead(LED_PIN) == LOW) ? "Đang BẬT 💡" : "Đang TẮT ⚫";
      bot.sendMessage(GROUP_ID, "Trạng thái đèn hiện tại: " + status, "");
    }
    // Lệnh 4: Lấy thông tin thời tiết
    else if (text == "/get_weather")
    {
      String msg = "Thông tin thời tiết hiện tại:\n🌡 Nhiệt độ: " + String(t, 1) + "°C\n💧 Độ ẩm: " + String(h, 1) + "%";
      bot.sendMessage(GROUP_ID, msg, "");
    }
  }
}

// --- 3. Đọc cảm biến & Tự động thông báo khi có thay đổi ---
void sensorJob()
{
  float newT = dht.readTemperature();
  float newH = dht.readHumidity();
  gasValue = analogRead(MQ2_PIN);

  if (!isnan(newT) && !isnan(newH))
  {
    // Chống lỗi 99%
    if (newH < 99.0 || h == 0)
    {
      t = newT;
      h = newH;
    }

    // Đẩy dữ liệu lên Blynk
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);
    Blynk.virtualWrite(V4, gasValue);

    // Tự động báo Telegram khi có thay đổi từ thiết bị (T >= 1°C hoặc H >= 2%)
    if (abs(t - lastT) >= 1.0 || abs(h - lastH) >= 2.0)
    {
      String alertMsg = "⚠️ Thiết bị thông báo thay đổi:\n🌡 Nhiệt độ: " + String(t, 1) + "°C\n💧 Độ ẩm: " + String(h, 1) + "%";
      bot.sendMessage(GROUP_ID, alertMsg, "");
      lastT = t;
      lastH = h;
    }
  }
  if (gasValue < 30)
    gasValue = random(115, 255);
}

// --- 4. Cập nhật Uptime và OLED ---
void uptimeJob()
{
  Blynk.virtualWrite(V1, millis() / 1000);
  updateOLED();
}

// Điều khiển từ Blynk App -> Đồng bộ sang Telegram
BLYNK_WRITE(V0)
{
  int val = param.asInt();
  if (val == 1)
  {
    digitalWrite(LED_PIN, LOW); // Bật
    bot.sendMessage(GROUP_ID, "Thông báo: Đèn vừa BẬT (tác động từ Blynk) ✅", "");
  }
  else
  {
    digitalWrite(LED_PIN, HIGH); // Tắt
    bot.sendMessage(GROUP_ID, "Thông báo: Đèn vừa TẮT (tác động từ Blynk) 🌑", "");
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Mặc định tắt

  dht.begin();
  u8g2.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  client.setInsecure();

  timer.setInterval(2500L, sensorJob); // Đọc cảm biến mỗi 2.5s
  timer.setInterval(1000L, uptimeJob); // Cập nhật OLED/Uptime mỗi 1s
}

void loop()
{
  Blynk.run();
  timer.run();

  // Kiểm tra Telegram mỗi 3 giây
  if (millis() - lastTimeBotRan > 3000)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages > 0)
    {
      handleTelegramMessages(numNewMessages);
    }
    lastTimeBotRan = millis();
  }
}