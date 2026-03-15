#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// ===== PIN CONFIG =====
#define LED_DIR D6
#define DHTPIN D3
#define DHTTYPE DHT11

// ===== OBJECT =====
DHT dht(DHTPIN, DHTTYPE);

// OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// ===== VARIABLES =====
float temperature;
float humidity;

// ===== HÀM VẼ KHUNG NÉT ĐỨT =====
void drawDashedFrame()
{
  for (int x = 0; x < 128; x += 6)
  {
    u8g2.drawLine(x, 0, x + 3, 0);
    u8g2.drawLine(x, 63, x + 3, 63);
  }

  for (int y = 0; y < 64; y += 6)
  {
    u8g2.drawLine(0, y, 0, y + 3);
    u8g2.drawLine(127, y, 127, y + 3);
  }
}

// ===== HIỆU ỨNG CHỮ CHẠY =====
void welcomeAnimation()
{
  const char *text = "Welcome Nguyen Ba Quy Dat";

  u8g2.setFont(u8g2_font_logisoso16_tr);
  int textWidth = u8g2.getStrWidth(text);

  for (int x = 128; x > -textWidth; x--)
  {
    u8g2.clearBuffer();
    drawDashedFrame();
    u8g2.drawStr(x, 40, text);
    u8g2.sendBuffer();
    delay(10);
  }
}

// ===== SETUP =====
void setup()
{
  Serial.begin(9600);

  pinMode(LED_DIR, OUTPUT);

  dht.begin();
  u8g2.begin();

  // ===== INTRO ANIMATION =====
  welcomeAnimation();
}

// ===== LOOP =====
void loop()
{
  // ===== LED BLINK =====
  digitalWrite(LED_DIR, HIGH);
  delay(500);
  digitalWrite(LED_DIR, LOW);
  delay(500);

  // ===== READ SENSOR =====
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("DHT error");
    return;
  }

  // ===== SERIAL MONITOR =====
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Hum: ");
  Serial.print(humidity);
  Serial.println(" %");

  // ===== OLED DISPLAY =====
  u8g2.clearBuffer();
  drawDashedFrame();

  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(20, 12, "ESP8266 SENSOR");

  u8g2.setCursor(10, 35);
  u8g2.print("Temp: ");
  u8g2.print(temperature);
  u8g2.print(" C");

  u8g2.setCursor(10, 55);
  u8g2.print("Hum : ");
  u8g2.print(humidity);
  u8g2.print(" %");

  u8g2.sendBuffer();

  delay(2000);
}