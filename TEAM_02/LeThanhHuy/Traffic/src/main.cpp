#include <Arduino.h>

// ================== KHAI BÁO CHÂN ==================
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

// ================== TRẠNG THÁI ĐÈN =================
enum TrafficState {
  RED,
  GREEN,
  YELLOW
};

TrafficState currentState = RED;

// ================== THỜI GIAN ======================
const unsigned long TIME_RED    = 5000; // 5s
const unsigned long TIME_GREEN  = 7000; // 7s
const unsigned long TIME_YELLOW = 3000; // 3s

unsigned long previousMillis = 0;

// ==================================================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  // Trạng thái ban đầu
  previousMillis = millis();

  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  Serial.println("LED [RED   ] ON => 5 Seconds");
}

// ==================================================
void loop() {
  unsigned long currentMillis = millis();

  if (currentState == RED && currentMillis - previousMillis >= TIME_RED) {
    previousMillis = currentMillis;

    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, HIGH);
    currentState = GREEN;

    Serial.println("LED [GREEN ] ON => 7 Seconds");
  }

  else if (currentState == GREEN && currentMillis - previousMillis >= TIME_GREEN) {
    previousMillis = currentMillis;

    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW, HIGH);
    currentState = YELLOW;

    Serial.println("LED [YELLOW] ON => 3 Seconds");
  }

  else if (currentState == YELLOW && currentMillis - previousMillis >= TIME_YELLOW) {
    previousMillis = currentMillis;

    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED, HIGH);
    currentState = RED;

    Serial.println("LED [RED   ] ON => 5 Seconds");
  }
}