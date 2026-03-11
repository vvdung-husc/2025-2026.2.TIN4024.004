#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

#define BLYNK_TEMPLATE_ID "TMPL68m9JLug0"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "AewyQwihhtto3QI2ZJW9QcP9NVeS-eLO"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define BTN_LED 23
#define PIN_LED 21

#define CLK 18
#define DIO 19

#define DHTPIN 16
#define DHTTYPE DHT22

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

unsigned long currentMiliseconds = 0;
unsigned long uptime = 0;

bool blueButtonON = false;

bool IsReady(unsigned long &timer, uint32_t interval);
void updateButton();
void uptimeTask();
void readDHT();

void setup() {

  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(BTN_LED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin();

  Serial.println("Connecting WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  digitalWrite(PIN_LED, LOW);

  Blynk.virtualWrite(V3, blueButtonON);
  Blynk.virtualWrite(V0, uptime);   // gửi thời gian ban đầu

  Serial.println("=== START ===");
}

void loop() {

  Blynk.run();

  currentMiliseconds = millis();

  updateButton();
  uptimeTask();
  readDHT();
}

bool IsReady(unsigned long &timer, uint32_t interval) {

  if (currentMiliseconds - timer < interval) return false;

  timer = currentMiliseconds;
  return true;
}

void updateButton() {

  static unsigned long lastTime = 0;
  static int lastValue = HIGH;

  if (!IsReady(lastTime, 50)) return;

  int v = digitalRead(BTN_LED);

  if (v == lastValue) return;

  lastValue = v;

  if (v == LOW) return;

  blueButtonON = !blueButtonON;

  digitalWrite(PIN_LED, blueButtonON ? HIGH : LOW);

  Blynk.virtualWrite(V3, blueButtonON);

  if (blueButtonON) {

    uptime = 0;
    Blynk.virtualWrite(V0, uptime);

  } else {

    display.clear();
    uptime = 0;
    Blynk.virtualWrite(V0, uptime);

  }

  Serial.println(blueButtonON ? "LED ON" : "LED OFF");
}

void uptimeTask() {

  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  if (blueButtonON) {

    uptime++;                         // tăng mỗi 1 giây
    display.showNumberDec(uptime);    // hiển thị lên TM1637
    Blynk.virtualWrite(V0, uptime);   // gửi lên điện thoại

  }

}

void readDHT() {

  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 2000)) return;

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT read failed");
    return;
  }

  Serial.print("Temp: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
}

BLYNK_WRITE(V3) {

  blueButtonON = param.asInt();

  digitalWrite(PIN_LED, blueButtonON ? HIGH : LOW);

  if (blueButtonON) {

    uptime = 0;
    Blynk.virtualWrite(V0, uptime);

  } else {

    display.clear();
    uptime = 0;
    Blynk.virtualWrite(V0, uptime);

  }

  Serial.println(blueButtonON ? "Blynk -> LED ON" : "Blynk -> LED OFF");
}