 /*
THÔNG TIN NHÓM 3
1. Nguyễn Đình Tuấn
2. Nguyễn Đăng Hưng
3. 
4. 
5.
*/

#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN 16
#define DHTTYPE DHT22

#define LED_RED 4
#define LED_YELLOW 2
#define LED_GREEN 15

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void updateStatus();
void updateOLED();
void updateLED();

unsigned long lastDhtRead = 0;
unsigned long lastBlink = 0;

const unsigned long dhtInterval = 2000;   // đọc DHT mỗi 2s
const unsigned long blinkInterval = 500;  // LED nhấp nháy 0.5s

bool ledState = false;

float temperature = 0;
float humidity = 0;
String statusText = "";
uint8_t activeLed = 0; // 0-none, 1-green, 2-yellow, 3-red

void updateStatus() {
  if (temperature < 13) {
    statusText = "TOO COLD";
    activeLed = 1;
  } else if (temperature < 20) {
    statusText = "COLD";
    activeLed = 1;
  } else if (temperature < 25) {
    statusText = "COOL";
    activeLed = 2;
  } else if (temperature < 30) {
    statusText = "WARM";
    activeLed = 2;
  } else if (temperature < 35) {
    statusText = "HOT";
    activeLed = 3;
  } else {
    statusText = "TOO HOT";
    activeLed = 3;
  }
}

void updateLED() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  if (!ledState) return;

  if (activeLed == 1) digitalWrite(LED_GREEN, HIGH);
  if (activeLed == 2) digitalWrite(LED_YELLOW, HIGH);
  if (activeLed == 3) digitalWrite(LED_RED, HIGH);
}

void printConsole() {
  Serial.print(F("Temperature: "));
  Serial.println(statusText);

  Serial.print(temperature, 2);
  Serial.println(F(" °C"));

  Serial.print(F("Humidity: "));
  Serial.print(humidity, 2);
  Serial.println(F(" %"));

  Serial.println(F("------------------------"));
}

void updateOLED() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.println(statusText);

  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print(temperature, 2);
  display.print(" ");
  display.write(247); // ký hiệu độ ° trong ASCII
  display.print("C");

  display.setTextSize(1);
  display.setCursor(0, 38);
  display.println("Humidity:");

  display.setTextSize(2);
  display.setCursor(0, 48);
  display.print(humidity, 2);
  display.print(" %");

  display.display();
  printConsole(); 
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  Wire.begin(13, 12); // SDA, SCL 
  dht.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  unsigned long now = millis();

  // ---------- READ DHT (NON-BLOCKING) ----------
  if (now - lastDhtRead >= dhtInterval) {
    lastDhtRead = now;

    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    if (!isnan(temperature) && !isnan(humidity)) {
      updateStatus();
      updateOLED();
    }
  }
  if (now - lastBlink >= blinkInterval) {
    lastBlink = now;
    ledState = !ledState;
    updateLED();
  }
}