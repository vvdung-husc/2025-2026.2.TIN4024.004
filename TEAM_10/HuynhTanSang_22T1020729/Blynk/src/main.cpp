#define BLYNK_TEMPLATE_ID "TMPL6mkPLTu2s"
#define BLYNK_TEMPLATE_NAME "Blynk"
#define BLYNK_AUTH_TOKEN "JgbXsx6fm4-pBHTEPXARRJEeKe5i69ea"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define LED_PIN 21
#define BUTTON_PIN 23

#define DHTPIN 16
#define DHTTYPE DHT22

#define CLK 18
#define DIO 19

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

bool ledState = false;
bool lastButtonState = HIGH;

unsigned long startTime = 0;
unsigned long totalTime = 0;

void toggleLed() {

  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);

  if (ledState) {
    startTime = millis();
    Serial.println("Blynk -> Blue Light ON");
  } else {
    totalTime += millis() - startTime;
    Serial.println("Blynk -> Blue Light OFF");
  }

  Blynk.virtualWrite(V0, ledState);
}

BLYNK_WRITE(V0) {
  int value = param.asInt();

  if (value == 1 && !ledState) toggleLed();
  if (value == 0 && ledState) toggleLed();
}

void checkButton() {
  bool current = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && current == LOW) {
    toggleLed();
    delay(200);
  }

  lastButtonState = current;
}

void updateDisplay() {

  unsigned long timeNow = totalTime;

  if (ledState)
    timeNow += millis() - startTime;

  int seconds = timeNow / 1000;

  display.showNumberDec(seconds, true);
  Blynk.virtualWrite(V1, seconds);
}

void readDHT() {

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) return;

  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
}

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);

  dht.begin();
  delay(2000);

  Serial.println("System Ready...");

  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "128.199.144.129", 80);

  timer.setInterval(100L, checkButton);
  timer.setInterval(1000L, updateDisplay);
  timer.setInterval(2000L, readDHT);
}

void loop() {
  Blynk.run();
  timer.run();
}