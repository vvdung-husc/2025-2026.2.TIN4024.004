#define BLYNK_TEMPLATE_ID "TMPL62m8s-mHd"
#define BLYNK_TEMPLATE_NAME "BLYNK DHT"
#define BLYNK_AUTH_TOKEN "AkzENOVrDqenRMlowoa-bPdHDlPajTft"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN 21
#define BUTTON_PIN 4

#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

BlynkTimer timer;

BLYNK_WRITE(V1) {
  digitalWrite(LED_PIN, param.asInt());
}

void sendData() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    Blynk.virtualWrite(V2, temp);
    Blynk.virtualWrite(V3, hum);
  }

  int uptime = millis() / 1000;
  Blynk.virtualWrite(V0, uptime);

  display.showNumberDec(uptime);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(7);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendData);
}

void loop() {
  Blynk.run();
  timer.run();
}