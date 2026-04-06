#include <DHTesp.h>

#define DHT_PIN 16
#define RELAY_PIN 23
#define LED_PIN 15

DHTesp dht;

void setup() {
  Serial.begin(115200);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW); // ban đầu tắt
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  TempAndHumidity data = dht.getTempAndHumidity();
  float temp = data.temperature;

  Serial.print("Nhiet do: ");
  Serial.print(temp);
  Serial.println(" °C");

  // 🔥 ĐÚNG YÊU CẦU ĐỀ BÀI
  if (temp > 30) {
    digitalWrite(RELAY_PIN, HIGH); // bật quạt
    digitalWrite(LED_PIN, HIGH);   // LED sáng
    Serial.println("QUAT ON");
  } else {
    digitalWrite(RELAY_PIN, LOW);  // tắt quạt
    digitalWrite(LED_PIN, LOW);    // LED tắt
    Serial.println("QUAT OFF");
  }

  delay(2000);
}