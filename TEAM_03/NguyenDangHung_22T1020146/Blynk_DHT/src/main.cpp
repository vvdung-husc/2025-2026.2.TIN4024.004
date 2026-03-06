#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6NSII5z1Z"
#define BLYNK_TEMPLATE_NAME "BlynkDHT"
#define BLYNK_AUTH_TOKEN "_Kz4z5UZKGmuVbnLdT7fqhPIVfiWdvOf"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* PIN CONFIG */
#define btnBLED 23
#define pinBLED 21

#define CLK 18
#define DIO 19

#define DHTPIN 16
#define DHTTYPE DHT22

/* OBJECT */
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

bool blueButtonON = true;
unsigned long lastTime = 0;

/* TIMER */
BlynkTimer timer;

/* BUTTON FUNCTION */
void checkButton() {
  static int lastState = HIGH;

  int state = digitalRead(btnBLED);

  if (state != lastState) {
    delay(20);
    lastState = state;

    if (state == HIGH) {
      blueButtonON = !blueButtonON;

      digitalWrite(pinBLED, blueButtonON);

      Blynk.virtualWrite(V1, blueButtonON);

      if (!blueButtonON) {
        display.clear();
      }
    }
  }
}

/* UPTIME + DISPLAY */
void sendUptime() {

  unsigned long sec = millis() / 1000;

  Blynk.virtualWrite(V0, sec);

  if (blueButtonON) {
    display.showNumberDec(sec);
  }
}

/* READ DHT */
void readDHT() {

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {

    Blynk.virtualWrite(V2, temp);
    Blynk.virtualWrite(V3, hum);

    Serial.print("Temp: ");
    Serial.println(temp);

    Serial.print("Hum: ");
    Serial.println(hum);
  }
}

/* BLYNK BUTTON */
BLYNK_WRITE(V1) {

  blueButtonON = param.asInt();

  digitalWrite(pinBLED, blueButtonON);

  if (!blueButtonON) {
    display.clear();
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(7);

  dht.begin();

  Serial.println("Connecting WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, sendUptime);
  timer.setInterval(2000L, readDHT);
}

void loop() {

  Blynk.run();
  timer.run();

  checkButton();
}