#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

#define LED_BOARD D4 
#define LED_DIR D6     
#define DHTPIN D3
#define DHTTYPE DHT22
#define MQ2 A0

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

void setup() {

  pinMode(LED_BOARD, OUTPUT);
  pinMode(LED_DIR, OUTPUT);

  Serial.begin(115200);

  dht.begin();
  u8g2.begin();
}

void loop() {

  // DIR sáng -> LED board tắt
  digitalWrite(LED_DIR, HIGH);
  digitalWrite(LED_BOARD, HIGH);
  delay(500);

  // DIR tắt -> LED board sáng
  digitalWrite(LED_DIR, LOW);
  digitalWrite(LED_BOARD, LOW);
  delay(500);

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int gas = analogRead(MQ2);

  Serial.print("Temp: ");
  Serial.println(temp);
  Serial.print("Hum: ");
  Serial.println(hum);
  Serial.print("Gas: ");
  Serial.println(gas);

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.setCursor(0,15);
  u8g2.print("Temp:");
  u8g2.print(temp);

  u8g2.setCursor(0,35);
  u8g2.print("Hum:");
  u8g2.print(hum);

  u8g2.setCursor(0,55);
  u8g2.print("Gas:");
  u8g2.print(gas);

  u8g2.sendBuffer();

  delay(2000);
}