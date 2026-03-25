/*
THÔNG TIN NHÓM 4
1. 
2. 
3. 
4. Nguyễn Bá Quý Đạt - Telegram : Đạt Nguyễn
5. Nguyễn Thành Doanh - Telegram : Doanh
*/

#define BLYNK_TEMPLATE_ID "TMPL6OFwxQT0X"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "fx6qfdwQmVH-Amhcr8kb5kVcIVXMWnZg"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "DHT.h"

// --- Cấu hình chân kết nối (Theo diagram.json) ---
#define PIN_LED 5
#define PIN_DHT 12
#define PIN_GAS 32
#define DHTTYPE DHT22

// --- Cấu hình Telegram ---
#define BOT_TOKEN "8161835889:AAEfRs92rg80jkTCOpDUR7VdEpOIRNs9spQ"
#define CHAT_ID "-5275861628"

// --- Khởi tạo đối tượng ---
char ssid[] = "CNTT-MMT"; // Thay bằng WiFi của bạn
char pass[] = "13572468";

DHT dht(PIN_DHT, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

BlynkTimer timer;

float temp, hum, gasValue;
unsigned long lastTimeBotRan;
const unsigned long botRequestDelay = 1000; // 1 giây kiểm tra tin nhắn 1 lần

// --- Hàm xử lý lệnh Telegram ---
void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
      continue; // Chỉ nhận lệnh từ group chỉ định

    String text = bot.messages[i].text;

    if (text == "/led_on")
    {
      digitalWrite(PIN_LED, HIGH);
      Blynk.virtualWrite(V1, 1);
      bot.sendMessage(CHAT_ID, "Đã bật đèn LED!", "");
    }
    else if (text == "/led_off")
    {
      digitalWrite(PIN_LED, LOW);
      Blynk.virtualWrite(V1, 0);
      bot.sendMessage(CHAT_ID, "Đã tắt đèn LED!", "");
    }
    else if (text == "/led_status")
    {
      String status = digitalRead(PIN_LED) ? "ĐANG BẬT" : "ĐANG TẮT";
      bot.sendMessage(CHAT_ID, "Trạng thái đèn: " + status, "");
    }
    else if (text == "/get_weather")
    {
      String msg = "Nhiệt độ: " + String(temp) + "°C\n";
      msg += "Độ ẩm: " + String(hum) + "%";
      bot.sendMessage(CHAT_ID, msg, "");
    }
  }
}

// --- Đồng bộ trạng thái từ Blynk App về thiết bị ---
BLYNK_WRITE(V1)
{
  int value = param.asInt();
  digitalWrite(PIN_LED, value);
  String status = value ? "Bật" : "Tắt";
  bot.sendMessage(CHAT_ID, "Blynk đã thay đổi trạng thái đèn: " + status, "");
}

// --- Hàm đọc cảm biến và gửi dữ liệu ---
void sendSensorData()
{
  // 1. Uptime (V0)
  Blynk.virtualWrite(V0, millis() / 1000);

  // 2. Đọc DHT22 (V2, V3)
  float newH = dht.readHumidity();
  float newT = dht.readTemperature();

  if (!isnan(newH) && !isnan(newT))
  {
    if (abs(newT - temp) > 0.5 || abs(newH - hum) > 1.0)
    { // Chỉ gửi khi có thay đổi
      temp = newT;
      hum = newH;
      Blynk.virtualWrite(V2, temp);
      Blynk.virtualWrite(V3, hum);
    }
  }

  // 3. Đọc Gas MQ2 (V4)
  gasValue = analogRead(PIN_GAS);
  // Nếu không có cảm biến thật, Wokwi sẽ trả về giá trị mô phỏng từ thanh trượt
  Blynk.virtualWrite(V4, gasValue);
}

void setup()
{
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  dht.begin();

  // Cấu hình mạng và Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Cấu hình bảo mật cho Telegram
  client.setInsecure();

  // Thiết lập timer gửi dữ liệu mỗi 2 giây
  timer.setInterval(2000L, sendSensorData);

  bot.sendMessage(CHAT_ID, "Hệ thống đã sẵn sàng!", "");
}

void loop()
{
  Blynk.run();
  timer.run();

  // Kiểm tra tin nhắn Telegram định kỳ
  if (millis() > lastTimeBotRan + botRequestDelay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}