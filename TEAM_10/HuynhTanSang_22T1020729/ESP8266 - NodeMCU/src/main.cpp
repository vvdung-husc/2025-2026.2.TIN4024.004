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

// ===== VARIABLE =====
unsigned long lastBlink = 0;
unsigned long lastRead = 0;

bool ledState = false;

float lastTemp = -100;
float lastHum = -100;
int lastGas = -1;

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  dht.begin();
  u8g2.begin();

  // ===== OLED GREETING =====
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(10, 30, "Xin chao");
  u8g2.drawStr(20, 55, "CNTT");
  u8g2.sendBuffer();

  delay(3000);   // hiển thị 3 giây
}

void loop()
{

  // ===== LED BLINK =====
  if (millis() - lastBlink > 1000)
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }

  // ===== READ SENSOR mỗi 3 giây =====
  if (millis() - lastRead > 3000)
  {
    lastRead = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int gasValue = analogRead(MQ2_PIN);

    // ===== TERMINAL UPDATE khi có thay đổi =====
    if (temperature != lastTemp || humidity != lastHum || gasValue != lastGas)
    {
      Serial.println("------ SENSOR UPDATE ------");

      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" C");

      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");

      Serial.print("Gas Value: ");
      Serial.println(gasValue);

      Serial.println("---------------------------");

      lastTemp = temperature;
      lastHum = humidity;
      lastGas = gasValue;
    }

    // ===== OLED DISPLAY =====
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_ncenB08_tr);

    u8g2.drawStr(0, 12, "ESP8266 MONITOR");

    u8g2.setCursor(0, 30);
    u8g2.print("Temp: ");
    u8g2.print(temperature);
    u8g2.print(" C");

    u8g2.setCursor(0, 45);
    u8g2.print("Hum : ");
    u8g2.print(humidity);
    u8g2.print(" %");

    u8g2.setCursor(0, 60);
    u8g2.print("Gas : ");
    u8g2.print(gasValue);

    u8g2.sendBuffer();
  }
}