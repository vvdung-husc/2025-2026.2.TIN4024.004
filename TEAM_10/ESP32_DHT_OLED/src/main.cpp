/*
THÔNG TIN NHÓM 10
1. Hoàng Anh Quân
2. Huỳnh Tấn Sang
3. Lê Nhữ Hoàng
4. Tôn Thất Bách
5. Trần Nguyễn Phước Kiệt
*/

#include  <Arduino.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ================= PIN CONFIG =================
#define LED_COLD   15   // led1
#define LED_MILD   2    // led2
#define LED_HOT    4    // led3

#define DHT_PIN    16
#define DHT_TYPE   DHT22

#define OLED_SDA   13
#define OLED_SCL   12

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= DHT =================
DHT dht(DHT_PIN, DHT_TYPE);

// ================= BLINK =================
unsigned long lastBlink = 0;
bool ledState = false;
const int blinkInterval = 1000;

// ================= WEATHER =================
String weatherStatus = "";

// =================================================
// SETUP
// =================================================
void setup() {
  Serial.begin(115200);

  pinMode(LED_COLD, OUTPUT);
  pinMode(LED_MILD, OUTPUT);
  pinMode(LED_HOT, OUTPUT);

  digitalWrite(LED_COLD, LOW);
  digitalWrite(LED_MILD, LOW);
  digitalWrite(LED_HOT, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while(1);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  dht.begin();
}

// =================================================
// WEATHER CHECK
// =================================================
int getWeatherLevel(float temp) {
  if (temp < 20) return 1;      // Cold group
  else if (temp < 30) return 2; // Mild group
  else return 3;                // Hot group
}

String getWeatherText(float temp) {
  if (temp < 13) return "TOO COLD	";
  else if (temp < 20) return "COLD";
  else if (temp < 25) return "COOL";
  else if (temp < 30) return "WARM";
  else if (temp < 35) return "HOT";
  else return "TOO HOT";
}

// =================================================
// OLED DISPLAY
// =================================================
void showOLED(float temp, float hum, String status) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Weather Monitor");

  display.setCursor(0,16);
  display.print("Temperature: ");
  display.print(temp);
  display.println(" C");

  display.setCursor(0,32);
  display.print("Humidity : ");
  display.print(hum);
  display.println(" %");

  display.setCursor(0,48);
  display.print("Status: ");
  display.println(status);

  display.display();
}

// =================================================
// LOOP
// =================================================
void loop() {

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT Error");
    return;
  }

  weatherStatus = getWeatherText(temp);
  int level = getWeatherLevel(temp);

  showOLED(temp, hum, weatherStatus);

  // ===== BLINK LOGIC NON BLOCK =====
  if (millis() - lastBlink >= blinkInterval) {
lastBlink = millis();
    ledState = !ledState;

    digitalWrite(LED_COLD, LOW);
    digitalWrite(LED_MILD, LOW);
    digitalWrite(LED_HOT, LOW);

    if (ledState) {
      if (level == 1) digitalWrite(LED_COLD, HIGH);
      if (level == 2) digitalWrite(LED_MILD, HIGH);
      if (level == 3) digitalWrite(LED_HOT, HIGH);
    }
  }
}