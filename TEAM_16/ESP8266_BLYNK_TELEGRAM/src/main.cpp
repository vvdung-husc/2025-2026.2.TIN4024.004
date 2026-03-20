/*
  THÔNG TIN NHÓM X
  1. Châu Kỳ
  2. Nguyễn Ái Danh
  3.
  4. 
  5. 
*/

#define BLYNK_TEMPLATE_ID "TMPL6haiWo-5w"
#define BLYNK_TEMPLATE_NAME "ESP32BlynkTelegram"
#define BLYNK_AUTH_TOKEN "hPZd_EqsL49KSzap970iPuu5L7ABRjq1"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR    0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= TELEGRAM =================
#define BOTtoken "8645992932:AAGNLi7s_jUyHqNFLO4ht-KFSOosxhpHq_4"
#define CHAT_ID "-5193448236"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ================= DHT =================
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ================= MQ2 =================
#define GAS_PIN 32   // Analog pin

// ================= LED =================
#define LED_PIN 5

// ================= BUZZER =================
#define BUZZER_PIN 15   // Nối DOUT của buzzer module

BlynkTimer timer;

// ================= BIẾN =================
float temp = 0;
float hum = 0;
int gasValue = 0;
bool gasAlertSent = false;

unsigned long startTime;

// Ngưỡng gas (hysteresis)
const int GAS_THRESHOLD_HIGH = 300;   // Kích hoạt alert
const int GAS_THRESHOLD_LOW  = 200;   // Tắt alert

// ================= ĐỌC CẢM BIẾN =================
void readSensor() {
  // Đọc DHT với retry
  float newTemp = dht.readTemperature();
  float newHum  = dht.readHumidity();

  if (isnan(newTemp) || isnan(newHum)) {
    Serial.println("Lỗi đọc DHT! Thử lại...");
    delay(500);
    newTemp = dht.readTemperature();
    newHum  = dht.readHumidity();
  }

  if (isnan(newTemp) || isnan(newHum)) {
    Serial.println("Vẫn lỗi DHT! Kiểm tra dây pull-up và kết nối.");
    temp = 0;
    hum = 0;
  } else {
    temp = newTemp;
    hum = newHum;
  }

  gasValue = analogRead(GAS_PIN);

  // Gửi lên Blynk
  Blynk.virtualWrite(V1, temp);     // Nhiệt độ
  Blynk.virtualWrite(V2, hum);      // Độ ẩm
  Blynk.virtualWrite(V3, gasValue); // Khí gas (0-4095)
  Blynk.virtualWrite(V4, (millis() - startTime) / 1000UL); // uptime giây

  // Cảnh báo gas với hysteresis
  if (gasValue > GAS_THRESHOLD_HIGH && !gasAlertSent) {
    String msg = "⚠️ CẢNH BÁO: Phát hiện KHÍ GAS! Giá trị = " + String(gasValue);
    bot.sendMessage(CHAT_ID, msg, "");
    gasAlertSent = true;
    digitalWrite(BUZZER_PIN, HIGH);  // Kêu buzzer (đổi LOW nếu active low)
  }
  else if (gasValue <= GAS_THRESHOLD_LOW && gasAlertSent) {
    gasAlertSent = false;
    digitalWrite(BUZZER_PIN, LOW);
    // Tùy chọn: bot.sendMessage(CHAT_ID, "✅ Khí gas đã giảm, an toàn!", "");
  }

  // Debug Serial
  Serial.printf("T: %.1f°C  H: %.1f%%  Gas: %d  Alert: %s\n", 
                temp, hum, gasValue, gasAlertSent ? "YES" : "NO");
}

// ================= HIỂN THỊ OLED =================
void displayOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("Team 16 - IoT");
  display.print("Nhiet do: "); display.print(temp, 1); display.println(" C");
  display.print("Do am:   ");  display.print(hum, 1);  display.println(" %");
  display.print("Gas:     ");  display.print(gasValue); display.println("");
  
  unsigned long secs = (millis() - startTime) / 1000;
  unsigned long hours = secs / 3600;
  unsigned long mins  = (secs % 3600) / 60;
  unsigned long s     = secs % 60;
  display.print("Uptime:  "); 
  display.printf("%lu:%02lu:%02lu", hours, mins, s);
  
  display.display();
}

// ================= TELEGRAM =================
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;
      String chat_id = String(bot.messages[i].chat_id);

      if (chat_id != CHAT_ID) continue;

      if (text == "/start") {
        String welcome = "Xin chào! Lệnh hỗ trợ:\n";
        welcome += "/led_on\n";
        welcome += "/led_off\n";
        welcome += "/led_status\n";
        welcome += "/get_weather";
        bot.sendMessage(CHAT_ID, welcome, "");
      }
      else if (text == "/led_on") {
        digitalWrite(LED_PIN, HIGH);
        Blynk.virtualWrite(V0, 1);
        bot.sendMessage(CHAT_ID, "LED đã BẬT", "");
      }
      else if (text == "/led_off") {
        digitalWrite(LED_PIN, LOW);
        Blynk.virtualWrite(V0, 0);
        bot.sendMessage(CHAT_ID, "LED đã TẮT", "");
      }
      else if (text == "/led_status") {
        String status = digitalRead(LED_PIN) ? "BẬT" : "TẮT";
        bot.sendMessage(CHAT_ID, "LED hiện tại: " + status, "");
      }
      else if (text == "/get_weather") {
        String msg = "Nhiệt độ: " + String(temp, 1) + " °C\n";
        msg += "Độ ẩm: " + String(hum, 1) + " %\n";
        msg += "Khí gas: " + String(gasValue);
        bot.sendMessage(CHAT_ID, msg, "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ================= BLYNK LED =================
BLYNK_WRITE(V0) {
  int val = param.asInt();
  digitalWrite(LED_PIN, val);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(27, 26);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC,OLED_ADDR)) {
    Serial.println(F("OLED init failed!"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Khoi dong...");
  display.display();

  dht.begin();
  delay(2000);  // Chờ DHT ổn định

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Telegram
  client.setInsecure();

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  startTime = millis();

  // Timer
  timer.setInterval(2000L, readSensor);
  timer.setInterval(1000L, handleTelegram);
  timer.setInterval(2000L, displayOLED);
}

// ================= LOOP =================
void loop() {
  Blynk.run();
  timer.run();
}