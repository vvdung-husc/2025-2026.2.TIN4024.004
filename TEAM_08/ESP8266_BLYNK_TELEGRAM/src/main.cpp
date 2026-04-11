/*
  THÔNG TIN NHÓM 8
  1. Trương Đức Mai Linh - Telegram: @linhtr020304
  2. Zơrâm Nhỏ - Telegram: ZoramNho97
  3. Phạm Hữu Anh Kiệt - Telegram: @Prophje
*/

#define BLYNK_TEMPLATE_ID "TMPL69uPnysdL"
#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "6Wf_PMqvwPnn3yFGwGGd04Y2DojetEHD"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>       
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CẤU HÌNH ---
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define BOT_TOKEN "8594921331:AAFZdT4p95pq-c-MCCL_1ccx20AvWIuJVyI"
#define CHAT_ID "-5294764327"

const char* weatherApiKey = "2a08745393f0d8eea6c9b5b2ab452271"; 
const char* city = "Hue";

#define DHTPIN 12
#define DHTTYPE DHT22
#define LED_PIN 5
#define GAS_PIN 34
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
BlynkTimer timer;

float temp, hum, weatherTemp = 0; 
int gasValue = 0;
bool ledState = false;

// --- HÀM HIỂN THỊ OLED ---
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("TEAM 08 - IOT");
  //display.printf("DHT: %.1fC - %d%%\n", temp, (int)hum);
  display.printf("Nhiet do: %.1f C\n", temp);
  display.printf("Do am: %d %%\n", (int)hum);
  display.printf("Gas: %d ppm\n", gasValue);
  display.printf("LED: %s\n", ledState ? "ON" : "OFF");
  //display.println("--------------------");
  display.display();
}

// --- LẤY THỜI TIẾT API ---
void fetchWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + String(weatherApiKey) + "&units=metric";
    if (http.begin(url)) {
      int httpCode = http.GET();
      if (httpCode > 0) {
        String payload = http.getString();
        StaticJsonDocument<1024> doc;
        deserializeJson(doc, payload);
        weatherTemp = doc["main"]["temp"];
        Blynk.virtualWrite(V6, weatherTemp);
      }
      http.end();
    }
  }
}

// --- XỬ LÝ TELEGRAM ---
void checkTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    // --- 1. LỆNH START: HIỂN THỊ MENU HƯỚNG DẪN ---
    if (text == "/start") {
      String welcome = "Sử dụng các lệnh sau để điều khiển đèn LED:\n";
      welcome += "Gửi /led_on : để bật đèn\n";
      welcome += "Gửi /led_off : để tắt đèn\n";
      welcome += "Gửi /led_status : để yêu cầu xem trạng thái đèn hiện tại\n";
      welcome += "Gửi /get_weather : để hiển thị nhiệt độ và độ ẩm";
      bot.sendMessage(chat_id, welcome, "");
    }

    // --- 2. LỆNH KIỂM TRA TRẠNG THÁI ĐÈN ---
    else if (text == "/led_status") {
      String state = ledState ? "Đèn LED đang bật" : "Đèn LED đang tắt";
      bot.sendMessage(chat_id, state, "");
    }

    // --- 3. ĐIỀU KHIỂN ĐÈN ---
    else if (text == "/led_on") {
      ledState = true; 
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, 1);
      bot.sendMessage(chat_id, "LED Đã Bật", "");
    } 
    else if (text == "/led_off") {
      ledState = false; 
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, 0);
      bot.sendMessage(chat_id, "LED Đã Tắt", "");
    }

    // --- 4. LỆNH LẤY THÔNG SỐ (GIỐNG ẢNH MẪU) ---
    else if (text == "/get_weather") {
      String msg = "Nhiệt độ: " + String(temp, 1) + " C\n";
      msg += "Độ ẩm: " + String(hum, 1) + " %";
      bot.sendMessage(chat_id, msg, "");

      // Gửi thêm dòng dữ liệu mới từ cảm biến (nếu muốn giống hệt ảnh 100%)
      String sensorMsg = "Dữ liệu mới từ cảm biến:\n";
      sensorMsg += "🌡 Nhiệt độ: " + String(temp, 1) + " C\n";
      sensorMsg += "💧 Độ ẩm: " + String(hum, 1) + " %";
      bot.sendMessage(chat_id, sensorMsg, "");
    }
  }
  updateOLED();
}

// --- BLYNK & SENSOR ---
BLYNK_WRITE(V1) { 
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
  updateOLED();
}

void sendSensorData() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  //gasValue = analogRead(GAS_PIN); 
  int rawGas = analogRead(GAS_PIN); 
  gasValue = map(rawGas, 0, 4095, 0, 1000);

  if (isnan(temp) || isnan(hum)) return;

  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V4, gasValue);
  
  // IN RA TERMINAL ĐỂ KIỂM TRA
  //Serial.printf("T: %.1f | H: %.1f | G: %d\n", temp, hum, gasValue);
  
  updateOLED();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED Failed");
  
  client.setInsecure(); 
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
  dht.begin();
  
  timer.setInterval(5000L, sendSensorData);    
  timer.setInterval(3000L, checkTelegram);     
  timer.setInterval(600000L, fetchWeather);    
  fetchWeather();
}

void loop() {
  Blynk.run();
  timer.run();
}