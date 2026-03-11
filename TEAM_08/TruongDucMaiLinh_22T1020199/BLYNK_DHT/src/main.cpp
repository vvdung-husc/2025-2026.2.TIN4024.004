#define BLYNK_TEMPLATE_ID "TMPL6zc48IliA"
#define BLYNK_TEMPLATE_NAME "BLYNK DHT"
#define BLYNK_AUTH_TOKEN "oi_sPOBjzqvNYcgXYxRx2pK8-EhIcF_o"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

#define LED_PIN 21
#define BTN_PIN 23
#define DHT_PIN 16
#define DHTTYPE DHT22

#define CLK 18
#define DIO 19

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

DHT dht(DHT_PIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

bool systemOn = false;
unsigned long secondsCount = 0;

int buttonState;
int lastButtonState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;


// ===== CẬP NHẬT TRẠNG THÁI HỆ THỐNG =====
void updateSystemState() {

  if (systemOn) {
    digitalWrite(LED_PIN, HIGH);
    display.showNumberDec(secondsCount);
  } else {
    digitalWrite(LED_PIN, LOW);
    display.clear();
  }

  Blynk.virtualWrite(V0, systemOn); // LED
}


// ===== ĐIỀU KHIỂN TỪ BLYNK =====
BLYNK_WRITE(V0) {

  systemOn = param.asInt();
  updateSystemState();

}


// ===== ĐẾM THỜI GIAN =====
void countTime() {

  if(systemOn){
    secondsCount++;
    display.showNumberDec(secondsCount);
  }

  Blynk.virtualWrite(V3, secondsCount); // ThoiGian
}


// ===== ĐỌC DHT22 =====
void sendSensor() {

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("Nhiet do: ");
  Serial.print(t);
  Serial.print(" C | Do am: ");
  Serial.print(h);
  Serial.println(" %");

  if (!isnan(t) && !isnan(h)) {

    Blynk.virtualWrite(V1, t); // NhietDo
    Blynk.virtualWrite(V2, h); // DoAm

  }
}


void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  digitalWrite(LED_PIN, LOW);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, countTime);
  timer.setInterval(2000L, sendSensor);
}


void loop() {

  Blynk.run();
  timer.run();

  int reading = digitalRead(BTN_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {

      buttonState = reading;

      if (buttonState == LOW) {

        systemOn = !systemOn;
        updateSystemState();

      }
    }
  }

  lastButtonState = reading;
}