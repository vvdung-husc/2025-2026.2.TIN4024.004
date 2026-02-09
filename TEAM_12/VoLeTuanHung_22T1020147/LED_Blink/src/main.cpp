#include <Arduino.h>

// #define PIN_LED_RED 23
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

//Non-blocking
// bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
//   if (millis() - ulTimer < millisecond) return false;
//   ulTimer = millis();
//   return true;
// }

// Hàm nháy LED
void blinkLED(int pin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
  }
}

void setup() {
  // put your setup code here, to run once:
  // printf("WELCOME IOT\n");
  // pinMode(PIN_LED_RED, OUTPUT);
 Serial.begin(115200);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
}

//Traffic Light arcoding to time
// void loop() {
//   // ===== RED =====
//   Serial.println("LED [RED   ] ON => 5 Seconds");
//   digitalWrite(PIN_LED_RED, HIGH);
//   delay(5000);
//   digitalWrite(PIN_LED_RED, LOW);

//   // ===== YELLOW =====
//   Serial.println("LED [YELLOW] ON => 3 Seconds");
//   digitalWrite(PIN_LED_YELLOW, HIGH);
//   delay(3000);
//   digitalWrite(PIN_LED_YELLOW, LOW);

//   // ===== GREEN =====
//   Serial.println("LED [GREEN ] ON => 7 Seconds");
//   digitalWrite(PIN_LED_GREEN, HIGH);
//   delay(7000);
//   digitalWrite(PIN_LED_GREEN, LOW);
// }

//Traffic Light arcoding to button press
void loop() {

  // 🔴 RED
  Serial.println("LED [RED   ] ON => 5 Seconds");
  blinkLED(PIN_LED_RED, 5);

  // 🟡 YELLOW
  Serial.println("LED [YELLOW] ON => 3 Seconds");
  blinkLED(PIN_LED_YELLOW, 3);

  // 🟢 GREEN
  Serial.println("LED [GREEN ] ON => 7 Seconds");
  blinkLED(PIN_LED_GREEN, 7);
}

//Non-Blocking
// void loop() {
//   static unsigned long ulTimer = 0;
//   static bool lesStatus = false;
//   if (IsReady(ulTimer, 500)){
//     lesStatus = !lesStatus;
//     printf("LES IS [%s]\n",lesStatus ? "ON" : "OFF");
//     digitalWrite(PIN_LED_RED, lesStatus ? HIGH : LOW);
//   }
// }

//BLOCKING
// void loop() {
//   // put your main code here, to run repeatedly:
//   // static int i = 0;
//   // printf("Loop running ...%d\n",++i);
//   // delay(1000);

//   digitalWrite(PIN_LED_RED, HIGH); // Turn LED ON
//   delay(500); // Wait for 500ms
//   digitalWrite(PIN_LED_RED, LOW); // Turn LED OFF
//   delay(500); // Wait for 500ms  
// }
