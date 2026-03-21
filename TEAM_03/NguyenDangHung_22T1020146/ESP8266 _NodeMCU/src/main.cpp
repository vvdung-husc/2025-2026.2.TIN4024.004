#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define LED_PIN D6
#define DHTPIN D4
#define DHTTYPE DHT11

LiquidCrystal_I2C lcd(0x27,16,2);
DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  Wire.begin(D2, D1);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("System Start");

  dht.begin();
  delay(2000);

  lcd.clear();
}

void loop() {

  // LED nhấp nháy
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  lcd.clear();

  if (isnan(h) || isnan(t)) {

    Serial.println("DHT read error");

    lcd.setCursor(0,0);
    lcd.print("DHT Error");

  } else {

    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print(" C  Hum: ");
    Serial.print(h);
    Serial.println(" %");

    lcd.setCursor(0,0);
    lcd.print("Temp:");
    lcd.print(t);
    lcd.print("C");

    lcd.setCursor(0,1);
    lcd.print("Hum:");
    lcd.print(h);
    lcd.print("%");
  }

  delay(1000);
}