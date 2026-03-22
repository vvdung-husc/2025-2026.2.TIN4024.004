#include <Arduino.h>
#include <DHT.h>
#include <TM1637Display.h>

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define CLK 19
#define DIO 18
TM1637Display display(CLK, DIO);

#define LED 2

int timeCount = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LED, OUTPUT);
  display.setBrightness(7);
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // hiển thị nhiệt độ
  display.showNumberDec((int)temp);

  // bật LED nếu >30 độ
  if (temp > 30) digitalWrite(LED, HIGH);
  else digitalWrite(LED, LOW);

  // giả lập thời gian hoạt động
  timeCount++;
  Serial.println(timeCount);

  delay(1000);
}