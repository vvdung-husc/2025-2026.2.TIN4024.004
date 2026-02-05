#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== DHT =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_GREEN 15
#define LED_YELLOW 2
#define LED_RED 4

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Blink =====
bool ledState = false;
unsigned long lastBlink = 0;
const int blinkInterval = 500;

// ===== Biến trạng thái =====
String statusText = "";
int ledMode = 0; 
// 1 = GREEN, 2 = YELLOW, 3 = RED

void controlLED() {
  // Tắt tất cả trước
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  // Nhấp nháy LED theo mode
  if (ledMode == 1) digitalWrite(LED_GREEN, ledState);
  if (ledMode == 2) digitalWrite(LED_YELLOW, ledState);
  if (ledMode == 3) digitalWrite(LED_RED, ledState);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  dht.begin();

  Wire.begin(13, 12);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while(true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT Error");
    return;
  }

  // ===== Xác định trạng thái =====
  if (temp < 13) {
    statusText = "TOO COLD";
    ledMode = 1;
  }
  else if (temp < 20) {
    statusText = "COLD";
    ledMode = 1;
  }
  else if (temp < 25) {
    statusText = "COOL";
    ledMode = 2;
  }
  else if (temp < 30) {
    statusText = "WARM";
    ledMode = 2;
  }
  else if (temp < 35) {
    statusText = "HOT";
    ledMode = 3;
  }
  else {
    statusText = "TOO HOT";
    ledMode = 3;
  }

  // ===== Blink LED =====
  if (millis() - lastBlink >= blinkInterval) {
    lastBlink = millis();
    ledState = !ledState;
    controlLED();
  }

  // ===== OLED =====
  display.clearDisplay();
  display.setCursor(0,0);

  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  display.print("Humidity: ");
  display.print(hum);
  display.println(" %");

  display.println();
  display.print("Status: ");
  display.println(statusText);

  display.display();

  delay(200);
}
