#include "main.h"

// ===== PIN CONFIG (THEO MẠCH BẠN GỬI) =====
#define PIN_LED_GREEN   15
#define PIN_LED_YELLOW  2
#define PIN_LED_RED     4
#define PIN_DHT         16

#define OLED_SDA 13
#define OLED_SCL 12

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DHT_OLED dhtSystem;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("DHT OLED READY");
  display.display();

  dhtSystem.setup(
    PIN_DHT,
    PIN_LED_GREEN,
    PIN_LED_YELLOW,
    PIN_LED_RED
  );
}

void loop() {
  dhtSystem.run(display);   // non-blocking
}