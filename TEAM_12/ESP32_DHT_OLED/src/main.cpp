/*
THÔNG TIN NHÓM 12
1. Lê Thị Thuỳ Linh
2. Nguyễn Tống Bảo Phúc
3. Võ Lê Tuấn Hưng
4.
5.
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

/* ================== OLED ================== */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ================== DHT22 ================= */
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

/* ================== LED =================== */
#define LED_GREEN  15
#define LED_YELLOW 2
#define LED_RED    4

/* ================== TIMING ================ */
unsigned long lastDHTRead = 0;
const unsigned long DHT_INTERVAL = 2000;   // 2s

unsigned long lastBlink = 0;
const unsigned long BLINK_INTERVAL = 500;  // 500ms ON / 500ms OFF
bool ledState = false;

/* ================== DATA ================== */
float temperature = 0.0;
float humidity = 0.0;
String statusText = "";

/* ============ FUNCTION PROTOTYPES ========= */
void updateStatus();
void blinkLED(unsigned long now);
void updateOLED();

/* ================== SETUP ================= */
void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  dht.begin();
  Wire.begin(13, 12); // SDA, SCL

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
}

/* ================== LOOP ================== */
void loop() {
  unsigned long now = millis();

  // ---- Read DHT22 (non-blocking, >=2s) ----
  if (now - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = now;
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    updateStatus();
  }

  blinkLED(now);
  updateOLED();
}

/* ============== STATUS LOGIC ============== */
void updateStatus() {
  if (temperature < 13)       statusText = "TOO COLD";
  else if (temperature < 20)  statusText = "COLD";
  else if (temperature < 25)  statusText = "COOL";
  else if (temperature < 30)  statusText = "WARM";
  else if (temperature < 35)  statusText = "HOT";
  else                        statusText = "TOO HOT";
}

/* ============== LED BLINK ================= */
void blinkLED(unsigned long now) {
  if (now - lastBlink >= BLINK_INTERVAL) {
    lastBlink = now;
    ledState = !ledState;
  }

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  if (temperature < 20) {
    digitalWrite(LED_GREEN, ledState);
  } 
  else if (temperature < 30) {
    digitalWrite(LED_YELLOW, ledState);
  } 
  else {
    digitalWrite(LED_RED, ledState);
  }
}

/* ============== OLED DISPLAY ============== */
void updateOLED() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.println(statusText);

  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print(temperature, 2);
  display.println(" C");

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.println("Humidity:");

  display.setTextSize(2);
  display.setCursor(0, 52);
  display.print(humidity, 2);
  display.println(" %");

  display.display();
}