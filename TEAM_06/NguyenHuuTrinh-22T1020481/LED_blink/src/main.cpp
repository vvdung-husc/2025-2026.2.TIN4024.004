#include <Arduino.h>
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32
#define LED_BLUE    21
#define BUTTON_PIN  23
#define CLK 18
#define DIO 19
const uint8_t digitMap[] = {
  0x3f, 0x06, 0x5b, 0x4f, 0x66,
  0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

enum State { WAIT, RED, YELLOW, GREEN };
State state = WAIT;
unsigned long lastTick = 0;
unsigned long blinkStart = 0;
bool blinking = false;
int blinkPin = -1;
int count = 0;
void tmStart() {
  digitalWrite(DIO, HIGH);
  digitalWrite(CLK, HIGH);
  digitalWrite(DIO, LOW);
}

void tmStop() {
  digitalWrite(CLK, LOW);
  digitalWrite(DIO, LOW);
  digitalWrite(CLK, HIGH);
  digitalWrite(DIO, HIGH);
}

void tmWriteByte(uint8_t b) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK, LOW);
    digitalWrite(DIO, b & 0x01);
    b >>= 1;
    digitalWrite(CLK, HIGH);
  }
  digitalWrite(CLK, LOW);
  pinMode(DIO, INPUT);
  digitalWrite(CLK, HIGH);
  pinMode(DIO, OUTPUT);
}

void tmDisplayNumber(int num) {
  tmStart();
  tmWriteByte(0x40);
  tmStop();

  tmStart();
  tmWriteByte(0xC0);
  tmWriteByte(digitMap[num]);
  tmWriteByte(0);
  tmWriteByte(0);
  tmWriteByte(0);
  tmStop();

  tmStart();
  tmWriteByte(0x88 | 0x07);
  tmStop();
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(CLK, OUTPUT);
  pinMode(DIO, OUTPUT);

  digitalWrite(LED_BLUE, LOW);
}

void loop() {
  unsigned long now = millis();
  if (state == WAIT) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      digitalWrite(LED_BLUE, HIGH);  
      state = RED;
      count = 5;
      lastTick = now;
    }
    return;
  }

  if (blinking && now - blinkStart >= 200) {
    digitalWrite(blinkPin, LOW);
    blinking = false;
  }

  if (now - lastTick >= 1000) {
    lastTick = now;
    if (state == RED) blinkPin = LED_RED;
    if (state == YELLOW) blinkPin = LED_YELLOW;
    if (state == GREEN) blinkPin = LED_GREEN;

    digitalWrite(blinkPin, HIGH);
    blinking = true;
    blinkStart = now;
    tmDisplayNumber(count);
    count--;

    if (count < 0) {
      if (state == RED) {
        state = YELLOW;
        count = 2;
      }
      else if (state == YELLOW) {
        state = GREEN;
        count = 5;
      }
      else if (state == GREEN) {
        state = RED;
        count = 5;
      }
    }
  }
}
