/*
	THÔNG TIN NHÓM 12
	1. Nguyễn Tống Bảo Phúc
	2. 
	3. 
  4.
  5.
*/

#define BLYNK_TEMPLATE_ID   "TMPL6ji_DGcOL"      // ← thay của bạn
#define BLYNK_TEMPLATE_NAME "BLYNK TELEGRAM IOT 12"
#define BLYNK_AUTH_TOKEN    "vNmtPIq3Ke7ASv9Y5YvgE4OBrVfx3l4r"       // ← thay của bạn

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ---- WiFi ----
const char* ssid     = "PARADOZX 9249";              
const char* password = "12345678";         

// ---- Telegram ----
#define BOT_TOKEN  "8791408418:AAEQhds2bxl0_y9DJhHSfkPKWhz-dik846M"
#define CHAT_ID    "-5129421151"

// ---- Chân phần cứng ----
#define DHT_PIN     D3
#define DHT_TYPE    DHT11
#define LED_RED     D6
#define LED_BLUE    D4
#define MQ2_PIN     A0

// ---- Đối tượng ----
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
BlynkTimer timer;

// ---- Biến toàn cục ----
float temperature = 0;
float humidity    = 0;
int   gasValue    = 0;
bool  ledState    = false;

// Gửi thông báo Telegram khi nhiệt độ/độ ẩm thay đổi đáng kể
float lastTempSent = -999;
float lastHumiSent = -999;

// =============================================
// Blynk: điều khiển LED qua Switch (V4)
// =============================================
BLYNK_WRITE(V4) {
  ledState = param.asInt();
  digitalWrite(LED_RED,  ledState ? HIGH : LOW);
  digitalWrite(LED_BLUE, ledState ? HIGH : LOW);
  Serial.println(ledState ? "LED ON" : "LED OFF");
}

// =============================================
// Đọc cảm biến & gửi lên Blynk
// =============================================
void sendSensorData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int   g = analogRead(MQ2_PIN);

  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity    = h;
  gasValue = g;

  // Gửi lên Blynk
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V3, gasValue);

  Serial.printf("Temp: %.1f°C | Humi: %.1f%% | Gas: %d\n",
                temperature, humidity, gasValue);

  // Gửi Telegram nếu thay đổi > 1°C hoặc > 2%
  if (abs(temperature - lastTempSent) >= 1.0 ||
      abs(humidity    - lastHumiSent) >= 2.0) {
    String msg = "🌡 Cập nhật cảm biến:\n";
    msg += "Nhiệt độ: " + String(temperature, 1) + " °C\n";
    msg += "Độ ẩm:    " + String(humidity,    1) + " %";
    bot.sendMessage(CHAT_ID, msg, "");
    lastTempSent = temperature;
    lastHumiSent = humidity;
  }
}

// =============================================
// Gửi Uptime lên Blynk (V0)
// =============================================
void sendUptime() {
  long uptimeSec = millis() / 1000;
  Blynk.virtualWrite(V0, uptimeSec);
}

// =============================================
// Xử lý lệnh Telegram từ chat
// =============================================
void handleTelegramMessages() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text    = bot.messages[i].text;
    String from    = bot.messages[i].from_name;  // tên người nhắn
    text.trim();

    Serial.println("Telegram: " + text);

    if (text == "/start") {
      String msg = "Xin chào, " + from + ".\n";
      msg += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      msg += "Gửi /led_on bật sáng đèn\n";
      msg += "Gửi /led_off để tắt đèn\n";
      msg += "Gửi /led_status để yêu cầu trạng thái đèn hiện tại\n";
      msg += "Gửi /get_weather để xem nhiệt độ & độ ẩm";
      bot.sendMessage(chat_id, msg, "");
    }
    else if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_RED,  HIGH);
      digitalWrite(LED_BLUE, HIGH);
      Blynk.virtualWrite(V4, 1);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_RED,  LOW);
      digitalWrite(LED_BLUE, LOW);
      Blynk.virtualWrite(V4, 0);
      bot.sendMessage(chat_id, "LED đã tắt", "");
    }
    else if (text == "/led_status") {
      String status = ledState ? "LED is ON" : "LED is OFF";
      bot.sendMessage(chat_id, status, "");
    }
    else if (text == "/get_weather") {
      String msg = "Nhiệt độ: " + String(temperature, 1) + " °C\n";
      msg += "Độ ẩm: "    + String(humidity,    1) + " %";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}


// =============================================
// SETUP
// =============================================
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED,  OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED,  LOW);
  digitalWrite(LED_BLUE, LOW);

  dht.begin();

  // Kết nối WiFi trước
  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK: " + WiFi.localIP().toString());

  // Bỏ qua xác thực SSL (dễ dùng cho học tập)
  secured_client.setInsecure();

  // Kết nối Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  // Đặt timer
  timer.setInterval(5000L,  sendSensorData);   // 5 giây đọc cảm biến
  timer.setInterval(10000L, sendUptime);        // 10 giây gửi uptime
  timer.setInterval(3000L,  handleTelegramMessages); // 3 giây check Telegram
}

// =============================================
// LOOP
// =============================================
void loop() {
  Blynk.run();
  timer.run();
}