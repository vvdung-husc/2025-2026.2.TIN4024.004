
	//THÔNG TIN NHÓM 2
	//1. Trần Hữu Tôn Hoàng Gia - Telegram : hoanggia21
	//2. Lê Thành Huy - Telegram : T.Huy


#define BLYNK_TEMPLATE_ID "TMPL6Y0XpCIbF"
#define BLYNK_TEMPLATE_NAME "Blynk Telegram"
#define BLYNK_AUTH_TOKEN "jzEwJeQjQEJNkwfoeTU2elGxHKb82qzr"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define BOT_TOKEN "8785416571:AAG4ezhAd6Q5qYrpY0gqQHZMOPFszoCXJbk"
#define CHAT_ID "-1003770400645"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// DHT
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// LED
#define LED_PIN 5
bool ledState = false;

// GAS
#define GAS_PIN 32

Adafruit_SSD1306 display(128, 64, &Wire);

BlynkTimer timer;

float temp = 0;
float hum = 0;
int gas = 0;
int gas_ppm = 0;

unsigned long startTime;

BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

void readSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) temp = t;
  if (!isnan(h)) hum = h;

  gas = analogRead(GAS_PIN);

  // quy đổi giả lập ppm (0–1000)
  gas_ppm = map(gas, 0, 4095, 0, 1000);

  // gửi Blynk
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V4, gas);

  // debug
  Serial.print("Gas ADC: ");
  Serial.print(gas);
  Serial.print(" | PPM: ");
  Serial.println(gas_ppm);

  // cảnh báo gas
  static bool sent = false;
  if (gas_ppm > 700 && !sent) {
    bot.sendMessage(CHAT_ID, "⚠️ CANH BAO: Khi gas cao!", "");
    sent = true;
  }
  if (gas_ppm < 500) sent = false;
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Uptime: ");
  display.println((millis() - startTime) / 1000);

  display.print("LED: ");
  display.println(ledState ? "ON" : "OFF");

  display.print("Temp: ");
  display.println(temp);

  display.print("Hum: ");
  display.println(hum);

  display.print("Gas: ");
  display.print(gas);
  display.println(" ADC");

  display.setCursor(0, 54);
  display.println("Team X");

  display.display();

  Blynk.virtualWrite(V0, (millis() - startTime) / 1000);
}

// ===== Telegram =====
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
      bot.sendMessage(CHAT_ID, "LED ON", "");
    }

    if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      ledState = false;
      bot.sendMessage(CHAT_ID, "LED OFF", "");
    }

    if (text == "/led_status") {
      bot.sendMessage(CHAT_ID, ledState ? "LED ON" : "LED OFF", "");
    }

    if (text == "/get_weather") {
      bot.sendMessage(CHAT_ID,
        "Temp: " + String(temp) + "C\nHum: " + String(hum) + "%",
        "");
    }

    if (text == "/gas") {
      bot.sendMessage(CHAT_ID,
        "Gas (ADC): " + String(gas),
        "");
    }
  }
}

// ===== setup =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  dht.begin();

  Wire.begin(27, 26);

  WiFi.begin(ssid, pass);
  client.setInsecure();

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  startTime = millis();

  timer.setInterval(2000L, readSensor);
  timer.setInterval(1000L, updateOLED);
}

// ===== loop =====
void loop() {
  Blynk.run();
  timer.run();
  handleTelegram();
}