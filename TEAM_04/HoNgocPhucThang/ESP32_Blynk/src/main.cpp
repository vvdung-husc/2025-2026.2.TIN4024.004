/***************************************************
 * ESP32 + Blynk + DHT22 + TM1637
 * Hoàn chỉnh theo Datastreams hiện tại
 ***************************************************/

// ===== BLYNK CONFIG =====
#define BLYNK_TEMPLATE_ID   "TMPL6ZgIbTt2f"
#define BLYNK_TEMPLATE_NAME "BlynkESP32"
#define BLYNK_AUTH_TOKEN    "4aR0in95OqZBFAZ1YuMGW9MBsp48cYM_"

// ===== LIBRARIES =====
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// ===== WIFI =====
char ssid[] = "VIETTEL";
char pass[] = "192.168.1.1";

// ===== PIN CONFIG =====
#define DHTPIN   13
#define DHTTYPE  DHT22

#define CLK      22
#define DIO      21

#define LED_GREEN 26

// ===== OBJECT =====
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

// ===== READ & SEND SENSOR =====
void sendDHT()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println("❌ DHT22 read failed");
    return;
  }

  Serial.printf("🌡 %.1f °C | 💧 %.1f %%\n", t, h);

  // Gửi đúng Datastream
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);

  // Hiển thị nhiệt độ lên TM1637
  display.showNumberDec((int)t, false);
}

// ===== BUTTON CONTROL (V3) =====
BLYNK_WRITE(V3)
{
  int state = param.asInt(); // 0 / 1
  digitalWrite(LED_GREEN, state);
}

// ===== SETUP =====
void setup()
{
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);

  display.setBrightness(7);
  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendDHT); // 2s
}

// ===== LOOP =====
void loop()
{
  Blynk.run();
  timer.run();
}