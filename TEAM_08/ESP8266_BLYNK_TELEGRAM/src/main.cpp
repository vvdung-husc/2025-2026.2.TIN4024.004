/*
  THÔNG TIN NHÓM 8
  1. Trương Đức Mai Linh
  2. Zơrâm Nhỏ
  3. Phạm Hữu Anh Kiệt
*/

// ================= BLYNK =================
#define BLYNK_TEMPLATE_ID "TMPL69uPnysdL"
#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "6Wf_PMqvwPnn3yFGwGGd04Y2DojetEHD"   

// ================= WIFI =================
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// ================= LIB =================
#include <WiFi.h>
#include <HTTPClient.h>          // Thêm thư viện này để gọi API thời tiết
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ================= API THỜI TIẾT =================
String apiKey = "5a5c4a9fec1b2237889827fd747858d6";
String lat = "16.4591"; // Vĩ độ (Ví dụ: Huế)
String lon = "107.5901"; // Kinh độ

float outdoorTemp = 0; // Biến lưu nhiệt độ ngoài trời từ API

// ================= TELEGRAM =================
#define BOT_TOKEN "8594921331:AAFZdT4p95pq-c-MCCL_1ccx20AvWIuJVyI"
#define CHAT_ID "-5294764327"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ================= DHT =================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ================= LED =================
#define LED_PIN 2
bool ledState = false;

// ================= TIMER =================
BlynkTimer timer;

float temp = 0;
float hum = 0;

// ================= BLYNK WRITE =================
BLYNK_WRITE(V1) {
  int value = param.asInt();
  digitalWrite(LED_PIN, value);
  ledState = value;
}

// ================= HÀM LẤY API THỜI TIẾT =================
void fetchWeather() {
  HTTPClient http;
  String url = "https://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&appid=" + apiKey + "&units=metric";
  
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc; // Cú pháp ArduinoJson v7
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      outdoorTemp = doc["main"]["temp"];
      
      // Gửi nhiệt độ ngoài trời lên Blynk (Ví dụ dùng chân V4)
      Blynk.virtualWrite(V4, outdoorTemp);
      
      Serial.print("Nhiệt độ ngoài trời (API): ");
      Serial.println(outdoorTemp);
    }
  }
  http.end();
}

// ================= TELEGRAM =================
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) continue;

    if (text == "/start") {
      bot.sendMessage(chat_id,
        "Commands:\n"
        "/led_on\n"
        "/led_off\n"
        "/led_status\n"
        "/get_weather", "");
    }

    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
      Blynk.virtualWrite(V1, 1);
      bot.sendMessage(chat_id, "LED ON", "");
    }

    if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      ledState = false;
      Blynk.virtualWrite(V1, 0);
      bot.sendMessage(chat_id, "LED OFF", "");
    }

    if (text == "/led_status") {
      bot.sendMessage(chat_id,
        ledState ? "LED ON" : "LED OFF", "");
    }

    if (text == "/get_weather") {
      // Cập nhật câu trả lời gộp cả 2 loại dữ liệu
      String msg = "🌡 Dữ liệu DHT22 (Trong nhà):\n";
      msg += "Temp: " + String(temp) + "°C\n";
      msg += "Hum: " + String(hum) + "%\n\n";
      
      msg += "🌤 Dữ liệu API (Ngoài trời):\n";
      msg += "Temp: " + String(outdoorTemp) + "°C";
      
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// ================= SENSOR + BLYNK =================
void sendSensor() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();

  // Gửi Blynk
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);

  // Uptime
  Blynk.virtualWrite(V0, millis()/1000);

  // LƯU Ý: Không nên gửi tin nhắn Telegram ở đây mỗi 3 giây, sẽ bị Telegram khóa Bot do spam.
  // Nếu muốn tự động gửi, hãy thiết lập chu kỳ dài hơn (ví dụ 1 tiếng/lần) hoặc chỉ gửi khi nhiệt độ thay đổi đột ngột.
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setInsecure(); // Cần thiết cho Telegram Bot hoạt động

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  dht.begin();

  // Khởi chạy API thời tiết lần đầu
  fetchWeather();

  // Thiết lập Timer
  timer.setInterval(3000L, sendSensor);     // Đọc DHT22 mỗi 3 giây
  timer.setInterval(60000L, fetchWeather);  // Đọc API Thời tiết mỗi 60 giây (1 phút)
}

// ================= LOOP =================
void loop() {
  Blynk.run();
  timer.run();

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}