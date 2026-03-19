	/*
	THÔNG TIN NHÓM 11
	1. Nguyễn Đình Hoàng
	2.
	3. ...
	*/
#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define BLYNK_TEMPLATE_ID "TMPL6noqEU9IH"
#define BLYNK_TEMPLATE_NAME "BlynkTelegram"
#define BLYNK_AUTH_TOKEN "brfwPR11A2U9dEk4dn3MWRdsvgcjBpBl"

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Cấu hình chân cắm (Khớp với diagram.json)
#define BTN_LED 23   // Nút bấm nối vào chân 23
#define PIN_LED 5    // LED nối vào chân 5 (qua điện trở)
#define CLK 18       // TM1637 CLK
#define DIO 19       // TM1637 DIO
#define DHTPIN 12    // DHT22 nối vào chân 12
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
  Blynk.syncVirtual(V3); 
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(BTN_LED, INPUT_PULLUP); // Sử dụng điện trở kéo lên nội bộ

  display.setBrightness(7);
  dht.begin();

  Serial.println("Connecting WiFi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  digitalWrite(PIN_LED, ledState);

  timer.setInterval(2000L, readDHT);
  timer.setInterval(1000L, updateUptime);
  timer.setInterval(50L, checkButton);
}

void loop() {
  Blynk.run();
  timer.run();
}

void readDHT() {
  if (!ledState) return;

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Lỗi: Không đọc được DHT22 tại chân 12!");
    return;
  }

  Serial.printf("Temp: %.1f, Hum: %.1f\n", t, h);

  Blynk.virtualWrite(V1, t); 
  Blynk.virtualWrite(V2, h); 
}

void updateUptime() {
  if (!ledState) return;
  uptime++;
  display.showNumberDec(uptime, true);
  Blynk.virtualWrite(V4, uptime); 
}

void checkButton() {
  static int lastState = HIGH;
  int v = digitalRead(BTN_LED);

  if (v != lastState) {
    delay(20); // Chống nhiễu nút bấm
    if (v == LOW) { // Nút được nhấn
      ledState = !ledState;
      digitalWrite(PIN_LED, ledState);
      Blynk.virtualWrite(V3, ledState);

      if (!ledState) {
        uptime = 0;
        display.clear();
      }
      Serial.println(ledState ? "Hệ thống: BẬT" : "Hệ thống: TẮT");
    }
    lastState = v;
  }
}

BLYNK_WRITE(V3) {
  ledState = param.asInt();
  digitalWrite(PIN_LED, ledState);
  if (!ledState) {
    uptime = 0;
    display.clear();
  }
}