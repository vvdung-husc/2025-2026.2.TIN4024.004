#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6kk58g6be"
#define BLYNK_TEMPLATE_NAME "LED BLYNK"
#define BLYNK_AUTH_TOKEN "lK7gMCcdNJgn1Qequ-8Dfh0wBHK68HLG"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// ========= WIFI =========
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ========= PIN =========
#define LED_PIN 21
#define DHT_PIN 16
#define DHTTYPE DHT22
#define CLK 18
#define DIO 19

// ========= OBJECT =========
DHT dht(DHT_PIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

// =================================================
// LED từ Blynk (V1)
// =================================================
BLYNK_WRITE(V1)
{
  digitalWrite(LED_PIN, param.asInt());
}

// =================================================
// Gửi nhiệt độ + độ ẩm
// =================================================
void sendSensor()
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) return;

  // gửi Blynk
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V0, h);

  // ===== HIỂN THỊ TM1637: 25.3 =====
  int temp = t * 10;
  display.showNumberDecEx(temp, 0b01000000, false); // bật dấu chấm
}

// =================================================
// Gửi uptime (V3 - STRING)
// =================================================
void sendUptime()
{
  unsigned long s = millis() / 1000;

  int hh = s / 3600;
  int mm = (s % 3600) / 60;
  int ss = s % 60;

  char buf[12];
  sprintf(buf, "%02d:%02d:%02d", hh, mm, ss);

  Blynk.virtualWrite(V3, buf);
}

// =================================================
// SETUP
// =================================================
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  dht.begin();

  display.setBrightness(7);
  display.clear();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensor);
  timer.setInterval(1000L, sendUptime);
}

// =================================================
// LOOP (bắt buộc)
// =================================================
void loop()
{
  Blynk.run();
  timer.run();
}