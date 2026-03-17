#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// ===== PIN =====
#define LED_PIN D6
#define DHTPIN  D3
#define MQ2_PIN A0

#define DHTTYPE DHT22

// ===== OBJECT =====
DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// ===== TIMER =====
unsigned long lastBlink = 0;
unsigned long lastRead = 0;

// ===== VARIABLE =====
bool ledState = false;

float temperature = 0;
float humidity = 0;
int gasValue = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  dht.begin();
  u8g2.begin();

  // ===== OLED GREETING =====
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(10, 30, "HELLO");
  u8g2.drawStr(20, 55, "CNTT");
  u8g2.sendBuffer();

  delay(3000);
}

void loop()
{

  // ===== LED BLINK =====
  if (millis() - lastBlink >= 1000)
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }

  // ===== READ SENSOR =====
  if (millis() - lastRead >= 3000)
  {
    lastRead = millis();

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    gasValue = analogRead(MQ2_PIN);

    // ===== CHECK ERROR DHT =====
    if (isnan(temperature) || isnan(humidity))
    {
      Serial.println("DHT ERROR");
      temperature = 0;
      humidity = 0;
    }

    // ===== SERIAL MONITOR =====
    Serial.println("------ SENSOR DATA ------");

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Gas Value: ");
    Serial.println(gasValue);

    Serial.println("-------------------------");

    // ===== OLED DISPLAY =====
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x12_tr);

    u8g2.drawStr(0, 10, "ESP8266 MONITOR");

    u8g2.setCursor(0, 25);
    u8g2.print("Temp: ");
    u8g2.print(temperature);
    u8g2.print(" C");

    u8g2.setCursor(0, 40);
    u8g2.print("Hum : ");
    u8g2.print(humidity);
    u8g2.print(" %");

    u8g2.setCursor(0, 55);
    u8g2.print("Gas : ");
    u8g2.print(gasValue);

    // ===== GAS WARNING =====
    if (gasValue > 400)
    {
      u8g2.drawStr(80, 55, "GAS!");
    }

    u8g2.sendBuffer();
  }
}