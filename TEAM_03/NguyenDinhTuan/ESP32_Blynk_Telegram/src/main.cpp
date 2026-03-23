#define BLYNK_TEMPLATE_ID "TMPL6lGFdsfCS"
#define BLYNK_TEMPLATE_NAME "IOT TEMPLATE"
#define BLYNK_AUTH_TOKEN "5YIFRBHjduRgj03jUZjm94APWi7rIPRs"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR    0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define BOTtoken "8788986390:AAESAKxGO7bQivRptRxYJ2xud1Uo484CbOA"
#define CHAT_ID "-1003797665941"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define GAS_PIN 32
#define LED_PIN 5
#define BUZZER_PIN 15

BlynkTimer timer;

float temp = 0;
float hum = 0;
int gasValue = 0;
bool gasAlertSent = false;

unsigned long uptimeSeconds = 0;
unsigned long lastTelegramPoll = 0;
const unsigned long TELEGRAM_POLL_MS = 200;  // poll every 200 ms

// Ngưỡng gas (hysteresis)
const int GAS_THRESHOLD_HIGH = 300;
const int GAS_THRESHOLD_LOW  = 200;

// ── Tick uptime every second & push to Blynk V1 ──────────────────────────────
void tickUptime() {
  uptimeSeconds++;
  Blynk.virtualWrite(V1, uptimeSeconds);  // 1, 2, 3 ...
}

// ── Read DHT + gas, push sensor data to Blynk ────────────────────────────────
void readSensor() {
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
    hum  = 0;
  } else {
    temp = newTemp;
    hum  = newHum;
  }

  gasValue = analogRead(GAS_PIN);

  // Push sensor data (V1/uptime is handled separately by tickUptime)
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V4, gasValue);

  // Gas alert with hysteresis
  if (gasValue > GAS_THRESHOLD_HIGH && !gasAlertSent) {
    String msg = "CẢNH BÁO: Phát hiện KHÍ GAS! Giá trị = " + String(gasValue);
    bot.sendMessage(CHAT_ID, msg, "");
    gasAlertSent = true;
    digitalWrite(BUZZER_PIN, HIGH);
  } else if (gasValue <= GAS_THRESHOLD_LOW && gasAlertSent) {
    gasAlertSent = false;
    digitalWrite(BUZZER_PIN, LOW);
    // bot.sendMessage(CHAT_ID, "✅ Khí gas đã giảm, an toàn!", "");
  }

  Serial.printf("T: %.1f°C  H: %.1f%%  Gas: %d  Uptime: %lus  Alert: %s\n",
                temp, hum, gasValue, uptimeSeconds, gasAlertSent ? "YES" : "NO");
}

// ── Update OLED every second (reuses already-updated uptimeSeconds) ───────────
void displayOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("Team 3 - IoT");
  display.print("Nhiet do: "); display.print(temp, 1); display.println(" C");
  display.print("Do am:   ");  display.print(hum, 1);  display.println(" %");
  display.print("Gas:     ");  display.println(gasValue);

  unsigned long m = (uptimeSeconds % 3600) / 60;
  unsigned long s = uptimeSeconds % 60;
  display.print("Uptime: ");
  display.printf("%02lu:%02lu", m, s);

  display.display();
}

// ── Handle Telegram commands ──────────────────────────────────────────────────
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text    = bot.messages[i].text;
      String chat_id = String(bot.messages[i].chat_id);

      if (chat_id != CHAT_ID) continue;

      if (text == "/start") {
        String welcome = "Xin chào! Lệnh hỗ trợ:\n/led_on\n/led_off\n/led_status\n/get_weather";
        bot.sendMessage(CHAT_ID, welcome, "");
      } else if (text == "/led_on") {
        digitalWrite(LED_PIN, HIGH);
        Blynk.virtualWrite(V0, 1);
        bot.sendMessage(CHAT_ID, "LED đã BẬT", "");
      } else if (text == "/led_off") {
        digitalWrite(LED_PIN, LOW);
        Blynk.virtualWrite(V0, 0);
        bot.sendMessage(CHAT_ID, "LED đã TẮT", "");
      } else if (text == "/led_status") {
        String status = digitalRead(LED_PIN) ? "BẬT" : "TẮT";
        bot.sendMessage(CHAT_ID, "LED hiện tại: " + status, "");
      } else if (text == "/get_weather") {
        String msg  = "Nhiệt độ: " + String(temp, 1) + " °C\n";
              msg += "Độ ẩm: "    + String(hum, 1)  + " %\n";
              msg += "Khí gas: "  + String(gasValue);
        bot.sendMessage(CHAT_ID, msg, "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ── Blynk V0 write handler ────────────────────────────────────────────────────
BLYNK_WRITE(V0) {
  digitalWrite(LED_PIN, param.asInt());
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN,    OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN,    LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(27, 26);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED init failed!"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Khoi dong...");
  display.display();

  dht.begin();
  delay(2000);

  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  client.setInsecure();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  // Three timers for sensors/display/uptime
  timer.setInterval(1000L, tickUptime);      // uptime: 1, 2, 3 … → V1
  timer.setInterval(1000L, readSensor);      // sensor + gas alert every 1 s
  timer.setInterval(1000L, displayOLED);     // OLED refreshes every 1 s
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  Blynk.run();
  timer.run();

  // Poll Telegram as fast as possible (every 200 ms) outside BlynkTimer
  // so commands are not gated behind the 1 s timer tick
  unsigned long now = millis();
  if (now - lastTelegramPoll >= TELEGRAM_POLL_MS) {
    lastTelegramPoll = now;
    handleTelegram();
  }
}