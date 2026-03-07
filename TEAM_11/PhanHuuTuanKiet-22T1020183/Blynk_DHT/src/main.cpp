
#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

#define BLYNK_TEMPLATE_ID "TMPL6EQg8owH7"
#define BLYNK_TEMPLATE_NAME "DHT"
#define BLYNK_AUTH_TOKEN "_2YLExirjuoMpVPKZrPbCnQSfhr1m8uW"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// LED + Button
#define BTN_LED 23
#define PIN_LED 21

// TM1637
#define CLK 18
#define DIO 19

// DHT22
#define DHTPIN 16
#define DHTTYPE DHT22

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

BlynkTimer timer;

bool ledState = true;
unsigned long uptime = 0;

void readDHT();
void updateUptime();
void checkButton();

BLYNK_CONNECTED() {
  Serial.println("Blynk connected");
  Blynk.syncVirtual(V1);
}

void setup() {

  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(BTN_LED, INPUT_PULLUP);

  display.setBrightness(7);

  dht.begin();

  Serial.println("Connecting WiFi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  digitalWrite(PIN_LED, ledState);

  Serial.println("=== START ===");

  timer.setInterval(2000L, readDHT);
  timer.setInterval(1000L, updateUptime);
  timer.setInterval(50L, checkButton);
}

void loop() {
  Blynk.run();
  timer.run();
}

void readDHT() {

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("DHT error");
    return;
  }

  Serial.print("Temp: ");
  Serial.println(t);

  Serial.print("Humidity: ");
  Serial.println(h);

  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);
}

void updateUptime() {

  if (!ledState) return;

  uptime++;

  display.showNumberDec(uptime, true);

  Blynk.virtualWrite(V0, uptime);
}

void checkButton() {

  static int lastState = HIGH;

  int v = digitalRead(BTN_LED);

  if (v == lastState) return;

  lastState = v;

  if (v == LOW) return;

  ledState = !ledState;

  digitalWrite(PIN_LED, ledState);

  Blynk.virtualWrite(V1, ledState);

  if (!ledState) {
    uptime = 0;
    display.clear();
  }

  Serial.println(ledState ? "LED ON" : "LED OFF");
}

BLYNK_WRITE(V1) {

  ledState = param.asInt();

  digitalWrite(PIN_LED, ledState);

  if (!ledState) {
    uptime = 0;
    display.clear();
  }

  Serial.println(ledState ? "Blynk -> LED ON" : "Blynk -> LED OFF");
}