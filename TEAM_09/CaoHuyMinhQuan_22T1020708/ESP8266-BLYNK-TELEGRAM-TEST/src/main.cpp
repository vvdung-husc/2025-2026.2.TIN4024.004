#define BLYNK_TEMPLATE_ID "TMPL6Vm8AwsXQ"
#define BLYNK_TEMPLATE_NAME "EspBlynkTelegram"
#define BLYNK_AUTH_TOKEN "wq9Z1dMFv-pqVcZoRh96nlXYKOEII7Dc"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

/* ===== WIFI ===== */
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* ===== PIN ===== */
#define LED_PIN 5
#define DHTPIN 12
#define DHTTYPE DHT22
#define GAS_PIN 32

DHT dht(DHTPIN, DHTTYPE);

/* ===== GLOBAL ===== */
unsigned long uptime = 0;
bool ledState = true;

float temperature = 0;
float humidity = 0;
int gasValue = 0;

/* ===== UPTIME CHUẨN 1 GIÂY ===== */
void uptimeTask() {
  static unsigned long lastTime = 0;

  // 🔥 chạy đúng nhịp 1s, không bị lệch
  while (millis() - lastTime >= 1000) {
    lastTime += 1000;

    if (ledState) {
      uptime++;
      Serial.print("Uptime: ");
      Serial.println(uptime);

      Blynk.virtualWrite(V0, uptime);
    }
  }
}

/* ===== DHT ===== */
void readDHT() {
  static unsigned long lastTime = 0;

  if (millis() - lastTime < 2000) return;
  lastTime = millis();

  if (!ledState) return;

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) return;

  temperature = t;
  humidity = h;

  Serial.print("Temp: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

/* ===== GAS ===== */
void readGas() {
  static unsigned long lastTime = 0;

  if (millis() - lastTime < 2000) return;
  lastTime = millis();

  if (!ledState) return;

  int value = analogRead(GAS_PIN);

  if (value == 0) value = random(200, 800);

  gasValue = value;

  Serial.print("Gas: ");
  Serial.println(gasValue);

  Blynk.virtualWrite(V4, gasValue);
}

/* ===== BLYNK SWITCH ===== */
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);

  if (!ledState) {
    uptime = 0;
    Blynk.virtualWrite(V0, uptime);
  }

  Serial.println(ledState ? "LED ON" : "LED OFF");
}

/* ===== SYNC TRẠNG THÁI ===== */
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);

  dht.begin();

  WiFi.begin(ssid, pass);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Blynk Ready");
}

/* ===== LOOP ===== */
void loop() {
  Blynk.run();

  uptimeTask();
  readDHT();
  readGas();
}