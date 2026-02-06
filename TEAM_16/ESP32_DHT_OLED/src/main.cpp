// THÔNG TIN NHÓM 16
// 1.Châu Kỳ
// 2.Nguyễn ÁI Danh
// 3 Châu Văn Trường Huy

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Cấu hình DHT22
#define DHTPIN 16
#define DHTTYPE DHT22

// LED
#define LED_GREEN   15
#define LED_YELLOW  2
#define LED_RED     4

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
#define OLED_ADDR    0x3C

// Thời gian
#define READ_INTERVAL 2000
#define BLINK_INTERVAL 300

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastRead = 0;
unsigned long lastBlink = 0;

bool ledState = false;
int activeLED = -1;

// Tắt toàn bộ LED
void turnOffLEDs() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
}

// Nhấp nháy LED không blocking
void blinkLED() {
  if (activeLED == -1) return;

  unsigned long now = millis();
  if (now - lastBlink >= BLINK_INTERVAL) {
    lastBlink = now;
    ledState = !ledState;
    digitalWrite(activeLED, ledState);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  turnOffLEDs();
  dht.begin();

  Wire.begin(13, 12);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void loop() {
  unsigned long now = millis();

  if (now - lastRead >= READ_INTERVAL) {
    lastRead = now;

    float temp = dht.readTemperature();
    float humi = dht.readHumidity();

    if (isnan(temp) || isnan(humi)) {
      Serial.println("DHT error");
      return;
    }

    display.clearDisplay();
    display.setCursor(0, 0);

    display.print("Temp: ");
    display.print(temp);
    display.println(" C");

    display.print("Humi: ");
    display.print(humi);
    display.println(" %");

    display.println("----------------");

    turnOffLEDs();
    activeLED = -1;
    ledState = false;

    // ===== XỬ LÝ THEO BẢNG NGƯỠNG =====

    if (temp < 13) {
      display.println("TOO COLD");
      display.println("GREEN LED");
      activeLED = LED_GREEN;
    }
    else if (temp < 20) {
      display.println("COLD");
      display.println("GREEN LED");
      activeLED = LED_GREEN;
    }
    else if (temp < 25) {
      display.println("COOL");
      display.println("YELLOW LED");
      activeLED = LED_YELLOW;
    }
    else if (temp < 30) {
      display.println("WARM");
      display.println("YELLOW LED");
      activeLED = LED_YELLOW;
    }
    else if (temp < 35) {
      display.println("HOT");
      display.println("RED LED");
      activeLED = LED_RED;
    }
    else {
      display.println("TOO HOT");
      display.println("RED LED");
      activeLED = LED_RED;
    }

    display.display();
  }

  blinkLED();
}
