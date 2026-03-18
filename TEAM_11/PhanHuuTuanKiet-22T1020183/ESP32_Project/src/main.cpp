#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

/* ------------ PIN CONFIG ------------ */

#define LED_PIN LED_BUILTIN
#define PIR_PIN D5

#define RELAY1 D8
#define RELAY2 D7

#define DHTPIN D3
#define DHTTYPE DHT22

#define MQ2_PIN A0

/* ------------ OBJECT ------------ */

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

/* ------------ TIMER ------------ */

unsigned long ledTimer = 0;
unsigned long sensorTimer = 0;

const int LED_INTERVAL = 500;
const int SENSOR_INTERVAL = 2000;

/* ------------ DATA ------------ */

bool ledState = false;

float temp = 0;
float hum = 0;

int pirState = 0;
int gasValue = 0;

/* ------------ SENSOR READ ------------ */

void readSensors()
{
  temp = dht.readTemperature();
  hum  = dht.readHumidity();

  pirState = digitalRead(PIR_PIN);

  gasValue = analogRead(MQ2_PIN);

  Serial.println("----- SENSOR DATA -----");

  if (isnan(temp) || isnan(hum))
  {
    Serial.println("DHT ERROR");
  }
  else
  {
    Serial.print("Temp : ");
    Serial.print(temp);
    Serial.println(" C");

    Serial.print("Hum  : ");
    Serial.print(hum);
    Serial.println(" %");
  }

  Serial.print("Gas  : ");
  Serial.println(gasValue);

  Serial.print("PIR  : ");
  Serial.println(pirState);

  Serial.println();
}

/* ------------ RELAY CONTROL ------------ */

void controlRelay()
{
  // PIR điều khiển Relay1
  if(pirState == HIGH)
      digitalWrite(RELAY1, LOW);
  else
      digitalWrite(RELAY1, HIGH);

  // Gas điều khiển Relay2
  if(gasValue > 600)
      digitalWrite(RELAY2, LOW);
  else
      digitalWrite(RELAY2, HIGH);
}

/* ------------ LED BLINK ------------ */

void blinkLED()
{
  if(millis() - ledTimer >= LED_INTERVAL)
  {
    ledTimer = millis();
    ledState = !ledState;

    digitalWrite(LED_PIN, ledState ? LOW : HIGH);
  }
}

/* ------------ OLED DISPLAY ------------ */

void displayOLED()
{
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x12_tf);

  u8g2.drawStr(0,12,"ESP8266 SMART SYSTEM");

  u8g2.setCursor(0,26);
  u8g2.print("Temp: ");
  u8g2.print(temp);
  u8g2.print(" C");

  u8g2.setCursor(0,38);
  u8g2.print("Hum : ");
  u8g2.print(hum);
  u8g2.print(" %");

  u8g2.setCursor(0,50);
  u8g2.print("Gas : ");
  u8g2.print(gasValue);

  u8g2.setCursor(0,62);
  u8g2.print("PIR : ");
  u8g2.print(pirState);

  u8g2.sendBuffer();
}

/* ------------ SETUP ------------ */

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  dht.begin();

  Wire.begin(D2, D1);

  u8g2.begin();

  Serial.println("ESP8266 SYSTEM START");
}

/* ------------ LOOP ------------ */

void loop()
{
  blinkLED();

  if(millis() - sensorTimer >= SENSOR_INTERVAL)
  {
    sensorTimer = millis();

    readSensors();

    controlRelay();

    displayOLED();
  }
}