#include <Arduino.h>
#include <TM1637Display.h>

/* Blynk Device Info */
#define BLYNK_TEMPLATE_ID "TMPL6ZgIbTt2f"
#define BLYNK_TEMPLATE_NAME "BlynkESP32"
#define BLYNK_AUTH_TOKEN "4aR0in95OqZBFAZ1YuMGW9MBsp48cYM_"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

#define DHTPIN 16
#define DHTTYPE DHT22

#define CLK 18
#define DIO 19

#define PIN_LED 21
#define PIN_BUTTON 23

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

bool isSystemOn = true;

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* --------- TIME (V0) ---------- */
void updateUptime() {

  if (!isSystemOn) return;

  long uptime = millis() / 1000;

  Blynk.virtualWrite(V0, uptime);

  display.showNumberDec(uptime);
}

/* --------- SENSOR (V1 V2) ---------- */
void sendSensorData() {

  if (!isSystemOn) return;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) return;

  Blynk.virtualWrite(V1, t);   // NhietDo
  Blynk.virtualWrite(V2, h);   // DoAm
}

/* --------- BUTTON WOKWI ---------- */
void checkPhysicalButton() {

  if (digitalRead(PIN_BUTTON) == LOW) {

    delay(200);

    isSystemOn = !isSystemOn;

    Blynk.virtualWrite(V3, isSystemOn);

    if (!isSystemOn) {

      display.clear();
      digitalWrite(PIN_LED, LOW);

    } else {

      digitalWrite(PIN_LED, HIGH);

    }

    Serial.print("Nut nhan: ");
    Serial.println(isSystemOn ? "ON" : "OFF");

    while (digitalRead(PIN_BUTTON) == LOW);
  }
}

/* --------- CONTROL BLYNK (V3) ---------- */
BLYNK_WRITE(V3) {

  isSystemOn = param.asInt();

  if (!isSystemOn) {

    display.clear();
    digitalWrite(PIN_LED, LOW);
    Serial.println("Blynk OFF");

  } else {

    digitalWrite(PIN_LED, HIGH);
    Serial.println("Blynk ON");

  }
}

void setup() {

  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  digitalWrite(PIN_LED, HIGH);

  dht.begin();

  display.setBrightness(0x0f);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(100L, checkPhysicalButton);
  timer.setInterval(1000L, updateUptime);
  timer.setInterval(2000L, sendSensorData);
}

void loop() {

  Blynk.run();
  timer.run();
}