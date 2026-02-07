/*
THÔNG TIN NHÓM 2
1. Trần Hữu Tôn Hoàng Gia
2. Lê Thành Huy
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ====== OLED ======
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

#define OLED_SDA 13
#define OLED_SCL 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ====== DHT ======
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ====== LED ======
#define LED_CYAN 15
#define LED_YELLOW 2
#define LED_RED 4

void setup() {
  Serial.begin(115200);

  // LED
  pinMode(LED_CYAN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // DHT
  dht.begin();

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED không khởi động được!");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Không đọc được DHT22");
    return;
  }

  // ====== OLED ======
  // ====== OLED (NEW LAYOUT) ======
display.clearDisplay();

// ----- Xac dinh trang thai nhiet do -----
String tempStatus;

if (temperature < 13) {
  tempStatus = "TOO COLD";
} else if (temperature < 20) {
  tempStatus = "COLD";
} else if (temperature < 25) {
  tempStatus = "COOL";
} else if (temperature < 30) {
  tempStatus = "WARM";
} else if (temperature <= 35) {
  tempStatus = "HOT";
} else {
  tempStatus = "TOO HOT";
}


// ----- Dong tieu de -----
display.setTextSize(1);
display.setCursor(0, 0);
display.print("Temperature: ");
display.println(tempStatus);

// ----- Nhiet do (to) -----
display.setTextSize(2);
display.setCursor(0, 12);
display.print(temperature, 2);
display.print(" ");
display.print((char)247); // ky hieu do C
display.print("C");

// ----- Do am -----
display.setTextSize(1);
display.setCursor(0, 40);
display.println("Humidity:");

display.setTextSize(2);
display.setCursor(0, 50);
display.print(humidity, 2);
display.print(" %");

display.display();


  // ====== LED theo nhiet do ======
  digitalWrite(LED_CYAN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  if (temperature < 20) {
    digitalWrite(LED_CYAN, HIGH);
  } else if (temperature <= 30) {
    digitalWrite(LED_YELLOW, HIGH);
  } else {
    digitalWrite(LED_RED, HIGH);
  }

  delay(500);

  digitalWrite(LED_CYAN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  delay(500);
}
