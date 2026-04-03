#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <ArduinoJson.h>

// --- Cấu hình ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
#define BOT_TOKEN "8658703604:AAEgrj6xBJLp7qGayZAe9wVk-_7LfnSF-zU"
#define CHAT_ID "-5117661455" 
#define LED_ON LOW
#define LED_OFF HIGH

// --- Khai báo chân cắm ---
#define MQ135_PIN 34
#define DHTPIN 14
#define DHTTYPE DHT22
#define LED_PIN 4 

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
WebServer server(80);

// --- Biến toàn cục ---
int airValue = 0;
float temp = 0.0, hum = 0.0, pm25 = 0.0;
bool ledStatus = false;
unsigned long lastTimeBotRan = 0;
String globalIP = "";

// --- Giao diện Web ---
String getHTML() {
  String html = "<!DOCTYPE html><html lang='vi'><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'><meta http-equiv='refresh' content='2'>";
  html += "<style>body{font-family:Arial,sans-serif;background:#f4f7f6;text-align:center;padding:20px;}";
  html += ".card{background:white;padding:15px;margin:10px auto;max-width:400px;border-radius:10px;box-shadow:0 4px 8px rgba(0,0,0,0.1);}";
  html += ".on{color:#2ecc71;} .off{color:#e74c3c;}</style></head><body>";
  html += "<h2>📊 TRUNG TÂM GIÁM SÁT</h2>";
  html += "<div class='card'>💨 Khí CO: " + String(airValue) + " PPM</div>";
  html += "<div class='card'>🌫️ Bụi PM2.5: " + String(pm25, 1) + " µg/m³</div>";
  html += "<div class='card'>🌡️ " + String(temp, 1) + "°C | 💧 " + String(hum, 1) + "%</div>";
  html += "<div class='card'>💡 Đèn: <b class='" + String(ledStatus ? "on" : "off") + "'>" + (ledStatus ? "ĐANG BẬT" : "ĐANG TẮT") + "</b></div>";
  html += "</body></html>";
  return html;
}

// --- Xử lý Telegram ---
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String chat_id = String(bot.messages[i].chat_id);

    if (text == "/get_status") {
      String msg = "🔍 **TRẠNG THÁI HỆ THỐNG**\n\n";
      msg += "💨 Khí CO: " + String(airValue) + " PPM\n";
      msg += "🌫️ Bụi PM2.5: " + String(pm25, 1) + " µg/m³\n";
      msg += "🌡️ Temp: " + String(temp, 1) + "°C\n";
      msg += "💧 Humid: " + String(hum, 1) + "%\n";
      msg += "💡 Đèn: " + String(ledStatus ? "ON" : "OFF") + "\n";
    msg += String("🌐 IP: ") + globalIP;
      bot.sendMessage(chat_id, msg, "Markdown");
    }
    else if (text == "/get_weather") {
      String msg = "🌡️ **THÔNG TIN THỜI TIẾT**\n\n";
      msg += "Nhiệt độ: " + String(temp, 1) + "°C\n";
      msg += "Độ ẩm: " + String(hum, 1) + "%";
      bot.sendMessage(chat_id, msg, "");
    }
    else if (text == "/led_on") {
      ledStatus = true;
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(chat_id, "💡 Đã bật đèn LED thành công!", "");
    }
    else if (text == "/led_off") {
      ledStatus = false;
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(chat_id, "🌑 Đã tắt đèn LED thành công!", "");
    }
    else if (text == "/start") {
      String welcome = "Chào Linh! Gõ các lệnh sau:\n";
      welcome += "/get_weather : Xem Nhiệt độ & Độ ẩm\n";
      welcome += "/get_status : Xem tất cả thông số\n";
      welcome += "/led_on : Bật đèn\n";
      welcome += "/led_off : Tắt đèn";
      bot.sendMessage(chat_id, welcome, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_ON);
  delay(2000);
  digitalWrite(LED_PIN, LED_OFF);

  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // ✅ Đặt ở đây mới đúng
  globalIP = WiFi.localIP().toString();
  Serial.println("\nWiFi Connected! IP: " + globalIP);

  client.setInsecure();
  server.on("/", []() { server.send(200, "text/html", getHTML()); });
  server.begin();

 // bot.sendMessage(String(CHAT_ID),
  //  "Hệ thống đã sẵn sàng!\n/get_weather\n/get_status\n/led_on\n/led_off", "");
}

void loop() {
  server.handleClient();
  static unsigned long lastMeasure = 0;
  if (millis() - lastMeasure > 2000) {
    airValue = analogRead(MQ135_PIN);
    pm25 = (airValue / 4095.0) * 150.0 + random(0, 3);
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) { temp = t; hum = h; }
    lastMeasure = millis();
  }

  if (millis() - lastTimeBotRan > 1000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}