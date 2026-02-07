#include <Arduino.h>
#include <TM1637Display.h>

/* ===== LED PINS ===== */
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    21

/* ===== DISPLAY ===== */
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

/* ===== INPUT ===== */
#define BUTTON_PIN 23
#define LDR_PIN    13
#define LUX_THRESHOLD 500

/* ===== BUTTON ===== */
volatile bool displayEnabled = true;
volatile bool buttonEvent = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

/* ===== YELLOW BLINK ===== */
unsigned long lastYellowToggle = 0;
bool yellowState = false;

/* ===== MODE STATE ===== */
bool lastNightMode = false;

/* ===== BUTTON ISR ===== */
void IRAM_ATTR handleButton() {
  unsigned long now = millis();
  if (now - lastInterruptTime > debounceDelay) {
    displayEnabled = !displayEnabled;
    buttonEvent = true;
    lastInterruptTime = now;
  }
}

/* ===== READ LUX (GIỮ ĐƠN GIẢN – KHÔNG IN RA) ===== */
int readLux() {
  int adc = analogRead(LDR_PIN);
  return map(adc, 0, 4095, 0, 1000);
}

/* ===== BLINK LED (NORMAL MODE) ===== */
bool blinkLed(int pin, int seconds, const char* name) {

  for (int i = seconds; i >= 0; i--) {

    // nếu chuyển sang night mode → thoát ngay
    if (readLux() > LUX_THRESHOLD) {
      digitalWrite(pin, LOW);
      display.clear();
      return false;
    }

    // === SERIAL GIỐNG CODE GỐC ===
    Serial.print("LED [");
    Serial.print(name);
    Serial.print("] => ");
    Serial.print(i);
    Serial.println(" seconds");

    if (displayEnabled) {
      display.showNumberDec(i, true);
    } else {
      display.clear();
    }

    digitalWrite(LED_BLUE, displayEnabled ? HIGH : LOW);

    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
  }

  display.clear();
  return true;
}

/* ===== YELLOW NIGHT MODE ===== */
void yellowNightMode() {
  unsigned long now = millis();

  if (now - lastYellowToggle >= 150) {
    yellowState = !yellowState;
    digitalWrite(LED_YELLOW, yellowState ? HIGH : LOW);
    lastYellowToggle = now;
  }
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(0x0f);
  display.clear();

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButton,
    FALLING
  );

  Serial.println("SYSTEM READY");
}

/* ===== LOOP ===== */
void loop() {

  bool nightMode = (readLux() > LUX_THRESHOLD);

  // ===== PHÁT HIỆN CHUYỂN MODE =====
  if (nightMode && !lastNightMode) {
    Serial.println("=== NIGHT MODE ===");
  }
  lastNightMode = nightMode;

  digitalWrite(LED_BLUE, displayEnabled ? HIGH : LOW);

  if (buttonEvent) {
    if (!nightMode) {
      Serial.println(displayEnabled ? "DISPLAY: ON" : "DISPLAY: OFF");
    }
    buttonEvent = false;
  }

  if (nightMode) {
    // ===== NIGHT MODE =====
    display.clear();
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);

    yellowNightMode();
    return;
  }

  // ===== NORMAL MODE =====
  yellowState = false;
  digitalWrite(LED_YELLOW, LOW);

  if (!blinkLed(LED_RED, 5, "RED")) return;
  delay(500);

  if (!blinkLed(LED_YELLOW, 3, "YELLOW")) return;
  delay(500);

  if (!blinkLed(LED_GREEN, 7, "GREEN")) return;
  delay(1000);
}