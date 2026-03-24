#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

#define DHTPIN D3
#define DHTTYPE DHT11
#define MQ2_PIN A0
#define LED_PIN D6

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0);

unsigned long previousLed = 0;
unsigned long previousSensor = 0;

const long ledInterval = 500;
const long sensorInterval = 2000;

float temp = 0;
float hum = 0;
int gas = 0;

void setup()
{
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);

  dht.begin();
  display.begin();

  Serial.println("System Start");
}

void loop()
{
  unsigned long currentMillis = millis();

  // LED Blink (non-blocking)
  if (currentMillis - previousLed >= ledInterval)
  {
    previousLed = currentMillis;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }

  // Read sensor
  if (currentMillis - previousSensor >= sensorInterval)
  {
    previousSensor = currentMillis;

    temp = dht.readTemperature();
    hum = dht.readHumidity();
    gas = analogRead(MQ2_PIN);

    if (isnan(temp) || isnan(hum))
    {
      Serial.println("DHT error!");
      return;
    }

    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print(" C  ");

    Serial.print("Hum: ");
    Serial.print(hum);
    Serial.print(" %  ");

    Serial.print("Gas: ");
    Serial.println(gas);

    display.clearBuffer();

    display.setFont(u8g2_font_ncenB08_tr);

    display.setCursor(0,15);
    display.print("Nhiet do: ");
    display.print(temp);
    display.print(" C");

    display.setCursor(0,35);
    display.print("Do am: ");
    display.print(hum);
    display.print(" %");

    display.setCursor(0,55);
    display.print("Gas: ");
    display.print(gas);

    display.sendBuffer();
  }
}