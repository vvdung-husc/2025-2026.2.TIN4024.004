#include <Arduino.h>

const int ledXanh = 25;
const int ledVang = 27;
const int ledDo = 26;

void nhapNhay(int pin, int soLan) {
  for (int i = 0; i < soLan; i++) {
    digitalWrite(pin, HIGH); // 1. Bật đèn
    delay(500);              // 2. Chờ 0.5 giây (Sáng)
    digitalWrite(pin, LOW);  // 3. TẮT ĐÈN (Quan trọng!)
    delay(500);              // 4. Chờ 0.5 giây (Tối)
  }
}

void setup() {
  pinMode(ledXanh, OUTPUT);
  pinMode(ledVang, OUTPUT);
  pinMode(ledDo, OUTPUT);

  Serial.begin(115200);
  Serial.println("--- BAT DAU HE THONG ---");
}

void loop() {
  // 1. ĐÈN XANH
  Serial.println("XANH dang nhap nhay...");
  nhapNhay(ledXanh, 7);

  // 2. ĐÈN VÀNG
  Serial.println("VANG dang nhap nhay...");
  nhapNhay(ledVang, 3);

  // 3. ĐÈN ĐỎ
  Serial.println("DO dang nhap nhay...");
  nhapNhay(ledDo, 10);
}