#include <Arduino.h>

// Khai báo chân LED
#define LED_RED     26
#define LED_YELLOW  33
#define LED_GREEN   32

void setup() {
  // Khởi động Serial Monitor
  Serial.begin(115200);

  // Khai báo LED là OUTPUT
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Tắt tất cả LED ban đầu
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  Serial.println("Bat dau chuong trinh LED tung buoc...");
}

void loop() {
  // Bước 1: LED đỏ
  Serial.println("Buoc 1: LED DO bat");
  digitalWrite(LED_RED, HIGH);
  delay(3000);
  digitalWrite(LED_RED, LOW);

  // Bước 2: LED vàng
  Serial.println("Buoc 2: LED VANG bat");
  digitalWrite(LED_YELLOW, HIGH);
  delay(3000);
  digitalWrite(LED_YELLOW, LOW);

  // Bước 3: LED xanh
  Serial.println("Buoc 3: LED XANH bat");
  digitalWrite(LED_GREEN, HIGH);
  delay(3000);
  digitalWrite(LED_GREEN, LOW);

  Serial.println("Hoan thanh 1 chu ky\n");
}
