/*
  THÔNG TIN NHÓM 13
1. Trần Tuấn Long
2. Nguyễn Thị Huyền Trân
3. Nguyễn Thị Thùy Ngân
4. Ngô Nghĩa
*/


#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RED_LED 4
#define YELLOW_LED 2
#define BLUE_LED 15
#define DHTPIN 16
#define DHTTYPE DHT22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastDHTRead = 0;
unsigned long lastLedBlink = 0;
const unsigned long DHT_INTERVAL = 2000;
const unsigned long LED_INTERVAL = 500;

float temperature = 0, humidity = 0;
void readDHT()
{
  if (millis() - lastDHTRead >= DHT_INTERVAL)
  {
    lastDHTRead = millis();
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
  }
}

bool ledState = false;
void LedBlink(int pin, int offPin1, int offPin2)
{
  digitalWrite(offPin1, LOW);
  digitalWrite(offPin2, LOW);
  if (millis() - lastLedBlink >= LED_INTERVAL)
  {
    lastLedBlink = millis();
    ledState = !ledState;
    digitalWrite(pin, ledState);
  }
}

void turnOffAllLed()
{
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
}

void handleTemperature()
{

  if (temperature < 13)
  {
    LedBlink(BLUE_LED, YELLOW_LED, RED_LED); // TOO COLD
  }
  else if (temperature < 20)
  {
    LedBlink(BLUE_LED, YELLOW_LED, RED_LED); // COLD
  }
  else if (temperature < 25)
  {
    LedBlink(YELLOW_LED, RED_LED, BLUE_LED); // COOL
  }
  else if (temperature < 30)
  {
    LedBlink(YELLOW_LED, RED_LED, BLUE_LED); // WARM
  }
  else if (temperature < 35)
  {
    LedBlink(RED_LED, YELLOW_LED, BLUE_LED); // HOT
  }
  else
  {
    LedBlink(RED_LED, YELLOW_LED, BLUE_LED);
  }
}
String getStatusText()
{
  if (temperature < 13)
    return "TOO COLD";
  else if (temperature < 20)
    return "COLD";
  else if (temperature < 25)
    return "COOL";
  else if (temperature < 30)
    return "WARM";
  else if (temperature < 35)
    return "HOT";
  else
    return "TOO HOT";
}
void updateOLED()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("DHT22 MONITOR");

  display.setCursor(0, 16);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 28);
  display.print("Humi: ");
  display.print(humidity, 1);
  display.println(" %");

  display.setCursor(0, 44);
  display.print("Status: ");
  display.println(getStatusText());

  display.display();
}
void setup()
{

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  dht.begin();
  Wire.begin(13, 12);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED FAILED");
    while (true);
  }
}
void loop()
{
  readDHT();           
  handleTemperature();
  updateOLED();
}