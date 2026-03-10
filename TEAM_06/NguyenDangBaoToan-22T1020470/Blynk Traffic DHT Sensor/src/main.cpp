#define BLYNK_TEMPLATE_ID "TMPL6YwR_13uq"
#define BLYNK_TEMPLATE_NAME "Blynk"
#define BLYNK_AUTH_TOKEN "49WHlx7_KgLV_3w4o05iA714MqRA1Bv4"
#toan
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define BTN_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define TM1637_CLK 18
#define TM1637_DIO 19

DHTesp dhtSensor;
TM1637Display display(TM1637_CLK, TM1637_DIO);
BlynkTimer timer;

unsigned long runtimeSeconds = 0;
bool ledState = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void updateDisplay() {
  display.showNumberDec(runtimeSeconds % 10000, true);
}

void sendSensorData() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  Serial.print("Temperature: ");
  Serial.print(data.temperature);
  Serial.print(" C | Humidity: ");
  Serial.println(data.humidity);

  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("Doc DHT22 that bai");
    return;
  }

  if (Blynk.connected()) {
    Blynk.virtualWrite(V2, data.temperature);
    Blynk.virtualWrite(V3, data.humidity);
  }
}

void updateRuntime() {
  runtimeSeconds++;
  updateDisplay();

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, runtimeSeconds);
  }
}

void checkPhysicalButton() {
  bool reading = digitalRead(BTN_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    static bool stableButtonState = HIGH;

    if (reading != stableButtonState) {
      stableButtonState = reading;

      if (stableButtonState == LOW) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);

        if (Blynk.connected()) {
          Blynk.virtualWrite(V1, ledState ? 1 : 0);
        }

        Serial.print("Nut nhan -> LED: ");
        Serial.println(ledState ? "ON" : "OFF");
      }
    }
  }

  lastButtonState = reading;
}

BLYNK_WRITE(V1) {
  int value = param.asInt();
  ledState = (value == 1);
  digitalWrite(LED_PIN, ledState);

  Serial.print("Blynk V1 -> LED: ");
  Serial.println(ledState ? "ON" : "OFF");
}

BLYNK_CONNECTED() {
  Serial.println("Blynk connected");
  Blynk.syncVirtual(V1);
  Blynk.virtualWrite(V0, runtimeSeconds);
  sendSensorData();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  display.setBrightness(7);
  display.showNumberDec(0, true);

  Serial.println("Dang ket noi WiFi va Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "sgp1.blynk.cloud", 80);

  timer.setInterval(1000L, updateRuntime);
  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(50L, checkPhysicalButton);
}

void loop() {
  Blynk.run();
  timer.run();
}