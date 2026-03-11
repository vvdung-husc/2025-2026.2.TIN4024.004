/*
THÔNG TIN NHÓM 04
1. Nguyễn Thành Doanh
2. Lê Ngọc Minh
3. Hồ Ngọc Phúc Thăng
4. Võ Hữu Lộc
5. Nguyễn Bá Quý Đạt
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

/* ===== OLED ===== */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ===== DHT ===== */
#define DHTPIN   16
#define DHTTYPE  DHT22
DHT dht(DHTPIN, DHTTYPE);

/* ===== LED ===== */
#define LED_RED     4
#define LED_YELLOW  2
#define LED_GREEN   15

/* ===== Struct trạng thái ===== */
struct WeatherState {
  String text;
  uint8_t led;
};

/* ===== Hàm xác định trạng thái ===== */
WeatherState getWeatherState(float t) {
  WeatherState state;

  if (t < 13) {
    state.text = "TOO COLD";
    state.led  = LED_GREEN;
  } 
  else if (t < 20) {
    state.text = "COLD";
    state.led  = LED_GREEN;
  } 
  else if (t < 25) {
    state.text = "COOL";
    state.led  = LED_YELLOW;
  } 
  else if (t < 30) {
    state.text = "WARM";
    state.led  = LED_YELLOW;
  } 
  else if (t <= 35) {
    state.text = "HOT";
    state.led  = LED_RED;
  } 
  else {
    state.text = "TOO HOT";
    state.led  = LED_RED;
  }

  return state;
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Wire.begin(13, 12);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED loi!");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 25);
  display.println("Khoi dong he thong...");
  display.display();
  delay(2000);
}

/* ===== LOOP ===== */
void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Khong doc duoc DHT");
    return;
  }

  WeatherState ws = getWeatherState(t);

  /* ---- Hien thi OLED ---- */
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp");

  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(t, 1);
  display.print(" C");

  display.setTextSize(1);
  display.setCursor(80, 15);
  display.print(ws.text);

  display.setCursor(0, 38);
  display.print("Humidity");

  display.setTextSize(2);
  display.setCursor(0, 48);
  display.print(h, 1);
  display.print(" %");

  display.display();

  /* ---- LED ---- */
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  for (int i = 0; i < 2; i++) {
    digitalWrite(ws.led, HIGH);
    delay(400);
    digitalWrite(ws.led, LOW);
    delay(400);
  }
}
