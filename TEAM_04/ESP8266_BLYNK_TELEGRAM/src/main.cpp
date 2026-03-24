/*
THÔNG TIN NHÓM X
1. Lê Ngọc Minh - Telegram : binhnv
2. Lê Nguyễn Hương Nguyên - Telegram : nguyen2019
3. ...
4. Nguyễn Bá Quý Đạt - Telegram : nguyenbaquydat
*/

#define BLYNK_TEMPLATE_ID "TMPL6OFwxQT0X"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "fx6qfdwQmVH-Amhcr8kb5kVcIVXMWnZg"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Thông tin WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Thông tin Telegram
#define BOTtoken "8161835889:AAEfRs92rg80jkTCOpDUR7VdEpOIRNs9spQ"
#define CHAT_ID "-5275861628"

// Cấu hình chân Pin (Dựa trên diagram.json)
#define LED_PIN 5
#define DHTPIN 12
#define DHTTYPE DHT22
#define GAS_PIN 32

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

float temp, hum;
int gasValue;
unsigned long lastBotRun;
const unsigned long botInterval = 1000; // Kiểm tra tin nhắn mỗi 1s

// --- CÁC HÀM XỬ LÝ ---

void sendSensorData() {
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  gasValue = analogRead(GAS_PIN);

  if (isnan(hum) || isnan(temp)) {
    Serial.println("Lỗi đọc cảm biến DHT!");
    return;
  }

  // Cập nhật lên Blynk
  Blynk.virtualWrite(V0, millis() / 1000); // Uptime (giây)
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V4, gasValue);

  Serial.printf("Temp: %.1f°C, Hum: %.1f%%, Gas: %d\n", temp, hum, gasValue);
}

// Điều khiển từ App Blynk (V1)
BLYNK_WRITE(V1) {
  int state = param.asInt();
  digitalWrite(LED_PIN, state);
  String msg = (state == HIGH) ? "Đèn đã BẬT qua Blynk" : "Đèn đã TẮT qua Blynk";
  bot.sendMessage(CHAT_ID, msg, "");
}

// Xử lý lệnh từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, HIGH);
      bot.sendMessage(chat_id, "Đèn LED đã được BẬT", "");
    }
    else if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, LOW);
      bot.sendMessage(chat_id, "Đèn LED đã được TẮT", "");
    }
    else if (text == "/led_status") {
      String status = digitalRead(LED_PIN) ? "Đang BẬT" : "Đang TẮT";
      bot.sendMessage(chat_id, "Trạng thái đèn: " + status, "");
    }
    else if (text == "/get_weather") {
      String weather = "Nhiệt độ: " + String(temp, 1) + "°C\n";
      weather += "Độ ẩm: " + String(hum, 1) + "%\n";
      weather += "Khí gas: " + String(gasValue);
      bot.sendMessage(chat_id, weather, "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();

  // Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Cấu hình Telegram client
  client.setInsecure(); // Bỏ qua xác thực SSL để tiết kiệm bộ nhớ cho ESP32

  // Thiết lập thời gian gửi dữ liệu (mỗi 2 giây)
  timer.setInterval(2000L, sendSensorData);
  
  bot.sendMessage(CHAT_ID, "Hệ thống đã sẵn sàng!", "");
}

void loop() {
  Blynk.run();
  timer.run();

  // Kiểm tra tin nhắn Telegram
  if (millis() > lastBotRun + botInterval) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastBotRun = millis();
  }
}