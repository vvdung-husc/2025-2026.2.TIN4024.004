#include <Arduino.h>
#include <TM1637Display.h>

// ====== KHAI BÁO CHÂN ======
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    22

#define BTN_PIN     23
#define LDR_PIN     13

#define CLK_PIN     18
#define DIO_PIN     19

// ====== TM1637 ======
TM1637Display display(CLK_PIN, DIO_PIN);

// ====== BIẾN TRẠNG THÁI ======
enum TrafficState { RED, GREEN, YELLOW };
TrafficState state = RED;

int redTime = 5;
int yellowTime = 2;
int greenTime = 5;

int counter = redTime;
unsigned long lastTick = 0;

bool blueLedState = false;
bool lastBtnState = HIGH;

// ====== SETUP ======
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pinMode(BTN_PIN, INPUT_PULLUP);

  display.setBrightness(5);
  display.clear();

  digitalWrite(LED_RED, HIGH);   // Bắt đầu đèn đỏ
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  Serial.println("TRAFFIC LIGHT STARTED");
}

// ====== LOOP ======
void loop() {
  // ====== NÚT NHẤN BẬT/TẮT LED XANH DƯƠNG ======
  bool btnState = digitalRead(BTN_PIN);
  if (btnState == LOW && lastBtnState == HIGH) {
    blueLedState = !blueLedState;
    digitalWrite(LED_BLUE, blueLedState);
    Serial.println(blueLedState ? "BLUE LED ON" : "BLUE LED OFF");
    delay(200); // chống dội
  }
  lastBtnState = btnState;

  // ====== ĐỌC LDR ĐỂ ĐIỀU CHỈNH TỐC ĐỘ ======
  int ldrValue = analogRead(LDR_PIN);
  int tickSpeed = map(ldrValue, 0, 4095, 300, 1200);

  // ====== LOGIC ĐÈN GIAO THÔNG ======
  if (millis() - lastTick >= tickSpeed) {
    lastTick = millis();

    display.showNumberDec(counter, true);
    counter--;

    if (counter < 0) {
      switch (state) {
        case RED:
          state = GREEN;
          counter = greenTime;
          digitalWrite(LED_RED, LOW);
          digitalWrite(LED_GREEN, HIGH);
          break;

        case GREEN:
          state = YELLOW;
          counter = yellowTime;
          digitalWrite(LED_GREEN, LOW);
          digitalWrite(LED_YELLOW, HIGH);
          break;

        case YELLOW:
          state = RED;
          counter = redTime;
          digitalWrite(LED_YELLOW, LOW);
          digitalWrite(LED_RED, HIGH);
          break;
      }
    }
  }
}
