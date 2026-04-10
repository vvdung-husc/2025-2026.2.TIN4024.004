#define BLYNK_TEMPLATE_ID "TMPL6wYTipVFO"
#define BLYNK_TEMPLATE_NAME "BaiThiTieuLuan"
#define BLYNK_AUTH_TOKEN ""

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
BlynkTimer timer;

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// PIN
#define LED_PIN 32
#define RELAY_PIN 4
#define LED_QUAT 2

bool den = 0;
bool quat = 0;

unsigned long startTime;


// ===== OLED =====
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.println("SMART HOME");

  display.print("Den: ");
  display.println(den ? "ON" : "OFF");

  display.print("Quat: ");
  display.println(quat ? "ON" : "OFF");

  unsigned long t = (millis() - startTime) / 1000;
  display.print("Uptime: ");
  display.print(t);
  display.println(" s");

  display.display();
}

// ===== gửi uptime =====
void sendUptime() {
  long uptime = millis() / 1000;
  Blynk.virtualWrite(V2, uptime);
}

// ===== BLYNK =====
BLYNK_WRITE(V0) {
  den = param.asInt();
  digitalWrite(LED_PIN, den);
  updateOLED();
}

BLYNK_WRITE(V1) {
  quat = param.asInt();
  digitalWrite(RELAY_PIN, quat);
  digitalWrite(LED_QUAT, quat);
  updateOLED();
}

void setup() {

  Serial.begin(115200);

  Wire.begin(27,26);

  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_QUAT, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAIL");
    while(1);
  }

  display.clearDisplay();
  display.display();

  Serial.println("Connecting Blynk...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Blynk Connected!");

  startTime = millis();
  timer.setInterval(1000L, sendUptime);

  updateOLED();
}

void loop() {
  Blynk.run();
  timer.run();
}