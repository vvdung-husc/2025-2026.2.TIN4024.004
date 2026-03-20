#define BLYNK_TEMPLATE_ID "TMPL6_csUe7yg"
#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK"
#define BLYNK_AUTH_TOKEN "W2XAjuD1wfeFBGTnkeMGbydPghieQdqN"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= THÔNG TIN WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";       

// ================= THÔNG TIN TELEGRAM =================
#define BOT_TOKEN "8395175272:AAGv_RXH4CylJGITAU-dAzCvf2wUCHuYjGQ" 
#define GROUP_CHAT_ID "-5216104297"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ================= CẤU HÌNH PHẦN CỨNG =================
// 1. Cấu hình LED
#define LED_PIN 5
bool ledState = false;

// 2. Cấu hình DHT22
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// 3. Cấu hình OLED (SDA=27, SCL=26)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 27
#define OLED_SCL 26
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 4. Cấu hình Cảm biến Khí Gas
#define GAS_PIN 32

// ================= BIẾN TOÀN CỤC =================
float lastTemp = 0.0;
float lastHum = 0.0;
unsigned long lastTimeBotRan = 0;
int botRequestDelay = 1000; 

unsigned long lastTimeDHTRead = 0;
int dhtReadDelay = 2000; 

// ================= CẬP NHẬT MÀN HÌNH OLED =================
// Đưa hàm này lên trên cùng để tránh lỗi "not declared in this scope" trong PlatformIO
void updateOLED() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int gasValue = analogRead(GAS_PIN);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0, 0);
  display.print("SMART HOME ESP32");
  
  display.setCursor(0, 15);
  display.print("Temp: ");
  display.print(isnan(t) ? "--" : String(t, 1));
  display.print(" C");

  display.setCursor(0, 25);
  display.print("Hum : ");
  display.print(isnan(h) ? "--" : String(h, 1));
  display.print(" %");

  display.setCursor(0, 35);
  display.print("Gas : ");
  display.print(gasValue);

  display.setCursor(0, 50);
  display.print("LED : ");
  display.print(ledState ? "ON" : "OFF");

  display.display();
}

// ================= XỬ LÝ LỆNH TỪ BLYNK =================
// V1 là Virtual Pin dùng để bật/tắt LED trên ứng dụng Blynk
BLYNK_WRITE(V1) {
  int pinValue = param.asInt();
  ledState = (pinValue == 1);
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  updateOLED();
}

// ================= XỬ LÝ LỆNH TỪ TELEGRAM =================
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
      bot.sendMessage(chat_id, "💡 Đèn LED đã được **BẬT**.", "");
      updateOLED();
    } 
    else if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      ledState = false;
      bot.sendMessage(chat_id, "🌑 Đèn LED đã được **TẮT**.", "");
      updateOLED();
    } 
    else if (text == "/led_status") {
      String status = ledState ? "ĐANG BẬT 💡" : "ĐANG TẮT 🌑";
      bot.sendMessage(chat_id, "Trạng thái LED hiện tại: " + status, "");
    } 
    else if (text == "/get_weather") {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      
      if (isnan(t) || isnan(h)) {
        bot.sendMessage(chat_id, "❌ Lỗi: Không thể đọc dữ liệu từ cảm biến DHT22!", "");
      } else {
        String msg = "🌤️ **Thông tin Thời tiết hiện tại:**\n";
        msg += "🌡️ Nhiệt độ: " + String(t, 1) + " °C\n";
        msg += "💧 Độ ẩm: " + String(h, 1) + " %";
        bot.sendMessage(chat_id, msg, "Markdown");
      }
    }
    else if (text == "/start") {
      String welcome = "Xin chào " + from_name + "!\n";
      welcome += "Hệ thống quản lý Smart Home đã sẵn sàng.\n\n";
      welcome += "Các lệnh hỗ trợ:\n";
      welcome += "/led_on - Bật đèn LED\n";
      welcome += "/led_off - Tắt đèn LED\n";
      welcome += "/led_status - Xem trạng thái LED\n";
      welcome += "/get_weather - Xem nhiệt độ, độ ẩm\n";
      bot.sendMessage(chat_id, welcome, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.println("Connecting WiFi...");
    display.display();
  }

  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  secured_client.setInsecure(); 

  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nĐã kết nối WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Khởi tạo Blynk sau khi đã kết nối WiFi
  Blynk.config(BLYNK_AUTH_TOKEN);

  updateOLED();
  
  lastTemp = dht.readTemperature();
  lastHum = dht.readHumidity();
  
  bot.sendMessage(GROUP_CHAT_ID, "✅ Hệ thống ESP32 đã khởi động và kết nối thành công!", "");
}

void loop() {
  // Duy trì kết nối với máy chủ Blynk
  Blynk.run();

  // 1. Kiểm tra tin nhắn từ Telegram
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("Nhận được lệnh từ Telegram");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // 2. Đọc cảm biến, gửi lên Blynk & tự động cảnh báo Telegram nếu thay đổi
  if (millis() - lastTimeDHTRead > dhtReadDelay) {
    float currentT = dht.readTemperature();
    float currentH = dht.readHumidity();
    int gasValue = analogRead(GAS_PIN);
    unsigned long uptimeStr = millis() / 1000; // Tính uptime bằng giây

    if (!isnan(currentT) && !isnan(currentH)) {
      updateOLED();

      // ----- GỬI DỮ LIỆU LÊN BLYNK -----
      Blynk.virtualWrite(V0, uptimeStr);         // V0: Uptime (s)
      Blynk.virtualWrite(V1, ledState ? 1 : 0);  // V1: Đồng bộ trạng thái Switch LED
      Blynk.virtualWrite(V2, currentT);          // V2: Nhiệt độ
      Blynk.virtualWrite(V3, currentH);          // V3: Độ ẩm
      Blynk.virtualWrite(V4, gasValue);          // V4: Khí gas từ MQ2

      // ----- CẢNH BÁO TELEGRAM NẾU CÓ THAY ĐỔI ĐỘT NGỘT -----
      if (abs(currentT - lastTemp) >= 0.5 || abs(currentH - lastHum) >= 2.0) {
        String alertMsg = "🔔 **CẬP NHẬT TRẠNG THÁI MÔI TRƯỜNG** 🔔\n";
        alertMsg += "Nhiệt độ mới: " + String(currentT, 1) + " °C\n";
        alertMsg += "Độ ẩm mới: " + String(currentH, 1) + " %";
        
        bot.sendMessage(GROUP_CHAT_ID, alertMsg, "Markdown");

        lastTemp = currentT;
        lastHum = currentH;
      }
    }
    lastTimeDHTRead = millis();
  }
}