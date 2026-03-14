#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// OLED
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// DHT
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ2
#define MQ2_PIN A0

#define LED LED_BUILTIN

float temperature;
float humidity;
int gasValue;

void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);

  dht.begin();
  u8g2.begin();
}

void loop() {

  // LED blink
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);

  // Read sensors
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  gasValue = analogRead(MQ2_PIN);

  // OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);

  u8g2.drawStr(0,10,"TRUONG DAI HOC");
  u8g2.drawStr(0,20,"KHOA HOC HUE");

  u8g2.setCursor(0,40);
  u8g2.print("Temp: ");
  u8g2.print(temperature);

  u8g2.setCursor(0,52);
  u8g2.print("Hum : ");
  u8g2.print(humidity);

  u8g2.setCursor(0,64);
  u8g2.print("Gas : ");
  u8g2.print(gasValue);

  u8g2.sendBuffer();

  delay(2000);
}