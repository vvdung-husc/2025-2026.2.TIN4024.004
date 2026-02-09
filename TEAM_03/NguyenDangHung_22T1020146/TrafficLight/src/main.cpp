#include <Arduino.h>
#include <TM1637Display.h>

// ===== PIN MAP THEO MẠCH =====
#define LED_GREEN   25
#define LED_YELLOW  26
#define LED_RED     27
#define LED_BLUE    21
#define BUTTON_PIN  23
#define LDR_PIN     13

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

#define RED_TIME     4
#define YELLOW_TIME  2
#define GREEN_TIME   6

int state = 0; 
int remainSeconds = GREEN_TIME;
bool blinkState = true;

bool displayOn = false;
bool lastButton = HIGH;

unsigned long lastSecond = 0;
unsigned long lastButtonRead = 0;

#define DARK_THRESHOLD 1500
#define SPEED 2

void setLight(int s) {
  digitalWrite(LED_GREEN, s == 0 ? blinkState : LOW);
  digitalWrite(LED_YELLOW, s == 1 ? blinkState : LOW);
  digitalWrite(LED_RED, s == 2 ? blinkState : LOW);
}

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(0x0f);
  display.clear();

  remainSeconds = GREEN_TIME;
}

void loop() {
  int lightValue = analogRead(LDR_PIN);
  bool isDark = lightValue > DARK_THRESHOLD;

  // ===== NÚT =====
  if (millis() - lastButtonRead > 30) {
    lastButtonRead = millis();
    bool btn = digitalRead(BUTTON_PIN);

    if (btn == LOW && lastButton == HIGH) {
      displayOn = !displayOn;
      digitalWrite(LED_BLUE, displayOn);
      if (!displayOn) display.clear();
    }
    lastButton = btn;
  }

  static unsigned long lastBlink = 0;
  static unsigned long lastSecondTick = 0;
  static bool localBlink = false;

  unsigned long now = millis();
  unsigned long secondInterval = 1000 / SPEED;

  // ===== BLINK 0.5s =====
  if (now - lastBlink >= secondInterval / 2) {
    lastBlink = now;
    localBlink = !localBlink;

    if (isDark) {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, localBlink);
    } else {
      blinkState = localBlink;
      setLight(state);
    }
  }

  if (!isDark && now - lastSecondTick >= secondInterval) {
  lastSecondTick = now;

  // HIỂN THỊ TRƯỚC
   if (displayOn) {
     display.showNumberDec(remainSeconds, true);
   }

  // SAU ĐÓ MỚI GIẢM
  remainSeconds--;

   if (remainSeconds < 0) {
     state = (state + 1) % 3;
     if (state == 0) remainSeconds = GREEN_TIME;
     if (state == 1) remainSeconds = YELLOW_TIME;
     if (state == 2) remainSeconds = RED_TIME;
   }
 }
}




