/*
THÔNG TIN NHÓM X
1. Nguyen Van A
2. Tran Van B
3. Le Van C
*/

// ===== BLYNK (PHẢI ĐẶT TRÊN) =====
#define BLYNK_TEMPLATE_ID "TMPL6z7SKHP-g"
#define BLYNK_TEMPLATE_NAME "ESP8622 Blynk Telegram"
#define BLYNK_AUTH_TOKEN "Rzi8cKhV3knsv2kXu48w8oGKRn8n7zj7"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// ===== WIFI =====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== TELEGRAM =====
#define BOT_TOKEN "8267739662:AAG3BRvRHVZemovCe_Sx8Yr5Ix4knh_WTUI"
#define CHAT_ID "8726144638"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ===== DHT =====
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== LED =====
#define LED_PIN 5

// ===== GAS =====
#define GAS_PIN 32

float temp, hum;
int gasValue;
bool ledState = false;

unsigned long lastTime = 0;
unsigned long lastTelegramCheck = 0;

// ===== BLYNK =====
BLYNK_WRITE(V0) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

// ===== TELEGRAM =====
void handleTelegram() {
  int numNewMsg = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMsg) {
    for (int i = 0; i < numNewMsg; i++) {
      String text = bot.messages[i].text;

      if (text == "/start") {
        bot.sendMessage(CHAT_ID,
          "Lenh:\n/led_on\n/led_off\n/led_status\n/get_data", "");
      }

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

      if (text == "/get_data") {
        String msg = "Temp: " + String(temp,1) + " C" +
                     "\nHum: " + String(hum,1) + " %" +
                     "\nGas: " + String(gasValue);
        bot.sendMessage(CHAT_ID, msg, "");
      }
    }
    numNewMsg = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  Wire.begin(27, 26);
  Wire.setClock(100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED fail");
    while (1);
  }

  // TEST OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("HELLO");
  display.display();
  delay(2000);

  dht.begin();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setInsecure();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// ===== LOOP =====
void loop() {
  Blynk.run();

  if (millis() - lastTime > 2000) {
    lastTime = millis();

    temp = dht.readTemperature();
    hum = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      temp = random(20, 35);
      hum = random(40, 80);
    }

    gasValue = analogRead(GAS_PIN);

    // Blynk
    Blynk.virtualWrite(V1, temp);
    Blynk.virtualWrite(V2, hum);
    Blynk.virtualWrite(V3, gasValue);

    // OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println("ESP32 IoT");

    display.print("Temp: ");
    display.print(temp,1);
    display.write(167); // ký tự °
    display.println("C");

    display.print("Hum: ");
    display.print(hum,1);
    display.println("%");

    display.print("Gas: ");
    display.println(gasValue);

    display.print("LED: ");
    display.println(ledState ? "ON" : "OFF");

    display.display();
  }

  if (millis() - lastTelegramCheck > 2000) {
    lastTelegramCheck = millis();
    handleTelegram();
  }
}