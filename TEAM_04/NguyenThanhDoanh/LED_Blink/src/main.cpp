 #include <Arduino.h>
void setup() {
  Serial.begin(115200);
  pinMode(23, OUTPUT);
  Serial.println("Hệ thống đã sẵn sàng!");
}

void loop() {
  digitalWrite(23, HIGH);
  delay(500);
  digitalWrite(23, LOW);
  delay(500);
}