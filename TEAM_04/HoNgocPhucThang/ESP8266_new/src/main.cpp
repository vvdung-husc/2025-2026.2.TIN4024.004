/*
THÔNG TIN NHÓM 4 - ĐÃ TỐI ƯU KẾT NỐI WIFI
*/

#define BLYNK_TEMPLATE_ID "TMPL6OFwxQT0X"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "fx6qfdwQmVH-Amhcr8kb5kVcIVXMWnZg"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "DHT.h"

// --- Cấu hình chân ---
#define PIN_LED D1
#define PIN_DHT D6
#define PIN_GAS A0
#define DHTTYPE DHT22

// --- Thông tin mạng (KIỂM TRA KỸ TẠI ĐÂY) ---
char ssid[] = "CNTT-MMT"; 
char pass[] = "13572468";

// --- Cấu hình Telegram ---
#define BOT_TOKEN "8161835889:AAEfRs92rg80jkTCOpDUR7VdEpOIRNs9spQ"
#define CHAT_ID "-5275861628"

DHT dht(PIN_DHT, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
BlynkTimer timer;

float temp, hum, gasValue;
unsigned long lastTimeBotRan;
const unsigned long botRequestDelay = 1000;

// Hàm gửi dữ liệu cảm biến
void sendSensorData() {
  float newH = dht.readHumidity();
  float newT = dht.readTemperature();
  gasValue = analogRead(PIN_GAS);

  if (!isnan(newH) && !isnan(newT)) {
    temp = newT;
    hum = newH;
    if (Blynk.connected()) { // Chỉ gửi khi đã kết nối Blynk
      Blynk.virtualWrite(V2, temp);
      Blynk.virtualWrite(V3, hum);
      Blynk.virtualWrite(V4, gasValue);
    }
  }
  
  // In ra Serial để bạn theo dõi dù chưa có mạng
  Serial.print("Nhiet do: "); Serial.print(temp);
  Serial.print(" | Do am: "); Serial.print(hum);
  Serial.print(" | Gas: "); Serial.println(gasValue);
}

void setup() {
  // 1. Khởi tạo Serial trước để debug
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- DANG KHOI DONG HE THONG ---");

  pinMode(PIN_LED, OUTPUT);
  dht.begin();

  // 2. Kết nối WiFi thủ công để kiểm soát lỗi
  Serial.print("Dang ket noi WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi da ket noi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // 3. Sau khi có WiFi mới khởi động Blynk và Telegram
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    
    client.setInsecure();
    configTime(7 * 3600, 0, "pool.ntp.org");
    bot.sendMessage(CHAT_ID, "ESP8266 đã trực tuyến!", "");
  } else {
    Serial.println("\nLoi: Khong the ket noi WiFi. Vui long kiem tra SSID/Pass.");
  }

  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
    
    // Kiểm tra tin nhắn Telegram
    if (millis() > lastTimeBotRan + botRequestDelay) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      // (Giữ nguyên logic handleNewMessages của bạn ở đây)
      lastTimeBotRan = millis();
    }
  }
  timer.run();
}