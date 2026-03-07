#include <Arduino.h>
#include <TM1637Display.h>

#define BLYNK_TEMPLATE_NAME "DHT Sensor"
#define BLYNK_AUTH_TOKEN "vSsAz6YR632tWXcOnpAFRaZ4PQLu5WDb"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>


char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ====== CHÂN ======
#define DHTPIN 16
#define DHTTYPE DHT22
#define LED_PIN 21
#define BUTTON_PIN 23
#define CLK 18
#define DIO 19
// ===================

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

unsigned long workingTime = 0;
bool isRunning = false;

// ===== Gửi nhiệt độ + độ ẩm =====
void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V2, t);  // V2 = Nhiệt độ
    Blynk.virtualWrite(V3, h);  // V3 = Độ ẩm
  }
}


// ===== Đếm thời gian =====
void countTime() {
  if (isRunning) {

    workingTime++;

    int minutes = (workingTime / 60) % 100;  // giới hạn 2 chữ số
    int seconds = workingTime % 60;

    int displayTime = minutes * 100 + seconds;

    // tránh vượt quá 9999
    if (displayTime > 9999) displayTime = 9999;

    display.showNumberDecEx(displayTime, 0b01000000, true);

    Blynk.virtualWrite(V1, workingTime);
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

// ===== Điều khiển LED từ Blynk =====
BLYNK_WRITE(V0) {
  int state = param.asInt();

  digitalWrite(LED_PIN, state);

  isRunning = state;   // LED bật thì chạy, tắt thì dừng
}

// ===== Start/Stop từ Blynk =====
BLYNK_WRITE(V4) {
  isRunning = param.asInt();
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  Serial.println("Start Program");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();

  display.setBrightness(7);
  display.showNumberDec(0);

  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Blynk connected");

  timer.setInterval(2000L, sendSensor);
  timer.setInterval(1000L, countTime);
}

// ===== Loop =====
void loop() {
  Blynk.run();
  timer.run();

  static bool lastState = HIGH;
  bool currentState = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && currentState == LOW) {
    isRunning = !isRunning;
    Blynk.virtualWrite(V4, isRunning);
    delay(200);
  }

  lastState = currentState;
}
