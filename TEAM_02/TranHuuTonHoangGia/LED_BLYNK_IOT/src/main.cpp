#define BLYNK_TEMPLATE_ID "TMPL6kk58g6be"
#define BLYNK_TEMPLATE_NAME "LED BLYNK"
#define BLYNK_AUTH_TOKEN "lK7gMCcdNJgn1Qequ-8Dfh0wBHK68HLG"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "YOUR_WIFI";
char pass[] = "YOUR_PASSWORD";

/* PIN */
#define LED_PIN 21
#define BTN_PIN 23
#define DHTPIN 16
#define CLK 18
#define DIO 19
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

/******** LED ********/
BLYNK_WRITE(V1) {
  int s = param.asInt();
  digitalWrite(LED_PIN, s);
  Serial.println("LED = " + String(s));
}

/******** BUTTON ********/
void checkButton() {
  static int last = HIGH;
  int now = digitalRead(BTN_PIN);

  if (last == HIGH && now == LOW) {
    int newState = !digitalRead(LED_PIN);
    digitalWrite(LED_PIN, newState);
    Blynk.virtualWrite(V1, newState);
  }
  last = now;
}

/******** SENSOR ********/
void sendSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.println("Temp: " + String(t));
  Serial.println("Hum: " + String(h));

  // ❗ luôn gửi (kể cả 0) để debug
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V0, h);

  display.showNumberDec((int)t);
}

/******** SETUP ********/
void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(7);

  WiFi.begin(ssid, pass);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected");

  Blynk.config(BLYNK_AUTH_TOKEN);

  if (Blynk.connect()) {
    Serial.println("Blynk Connected");
  } else {
    Serial.println("Blynk Failed");
  }

  timer.setInterval(2000L, sendSensor);
  timer.setInterval(100L, checkButton);

  digitalWrite(LED_PIN, LOW);
}

/******** LOOP ********/
void loop() {

  if (!Blynk.connected()) {
    Blynk.connect();
  }

  Blynk.run();
  timer.run();
}