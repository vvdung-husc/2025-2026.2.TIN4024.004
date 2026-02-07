#include <Arduino.h>
#include <TM1637Display.h>

// ====== PIN CONFIG ======
#define RED_LED 27
#define YELLOW_LED 26
#define GREEN_LED 25
#define BLUE_LED 21
#define BUTTON_PIN 23
#define LDR_PIN 13
#define LDR_THRESHOLD 1500 

#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// ====== SETTINGS ======
const unsigned long redTime = 5000;
const unsigned long greenTime = 7000;
const unsigned long yellowTime = 3000;
int lastState = 0;
bool isNight = false;
bool lastNightState = false;
// ====== VARIABLES ======
int state = 1; // 1=Red, 2=Green, 3=Yellow

unsigned long previousMillis = 0;
unsigned long blinkMillis = 0;
bool ledBlink = false;

bool systemRunning = false;
int lastDisplayedTime = -1;

// ====== RESET ======
void resetSystem()
{
  digitalWrite(BLUE_LED, LOW);
  display.clear();
  lastDisplayedTime = -1;
  Serial.println("--- DISPLAY OFF ---");
}

void checkLight()
{
  static unsigned long lastCheckMillis = 0;
  const unsigned long CHECK_INTERVAL = 500; // kiểm tra mỗi 500ms

  unsigned long now = millis();
  if (now - lastCheckMillis < CHECK_INTERVAL)
    return;

  lastCheckMillis = now;

  int ldrValue = analogRead(LDR_PIN);
  bool currentNight = (ldrValue > LDR_THRESHOLD);

  // Chỉ xử lý khi trạng thái DAY/NIGHT thay đổi
  if (currentNight != lastNightState)
  {
    isNight = currentNight;

    if (isNight)
    {
      Serial.println("====NIGHT====");
      state = 3;                 // Yellow
    }
    else
    {
      Serial.println("====DAY====");
      state = 1;                 // Red
    }

    previousMillis = now;        // reset timer đúng thời điểm
    lastDisplayedTime = -1;      // reset countdown
    lastNightState = isNight;
  }
}

void yellowBlink(unsigned long now){
  if (now - blinkMillis >= 500){
    blinkMillis = now;
    ledBlink = !ledBlink;
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, ledBlink);
  }
}
void setup()
{
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  previousMillis = millis();
}

// ====== BUTTON ======
void handleButton()
{
  static int lastReading = HIGH;
  static unsigned long lastDebounceTime = 0;
  static int buttonState = HIGH;

  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading)
    lastDebounceTime = millis();

  if (millis() - lastDebounceTime > 50)
  {
    if (reading != buttonState)
    {
      buttonState = reading;

      if (buttonState == LOW)
      {
        systemRunning = !systemRunning;

        if (systemRunning)
        {
          digitalWrite(BLUE_LED, HIGH);
          Serial.println("--- DISPLAY ON ---");
          lastDisplayedTime = -1;
        }
        else
        {
          resetSystem();
        }
      }
    }
  }
  lastReading = reading;
}

void trafficLight(unsigned long now)
{
  unsigned long duration = 0;
  const char *ledName = "";

  if (state == 1)
  {
    duration = redTime;
    ledName = "RED";
  }
  else if (state == 2)
  {
    duration = greenTime;
    ledName = "GREEN";
  }
  else if (state == 3)
  {
    duration = yellowTime;
    ledName = "YELLOW";
  }

  // ===== IN KHI VỪA ĐỔI ĐÈN =====
  if (state != lastState)
  {
    Serial.print("LED [");
    Serial.print(ledName);
    Serial.print("] ON => ");
    Serial.print(duration / 1000);
    Serial.println(" Seconds");

    lastState = state;
    lastDisplayedTime = -1; // reset countdown
  }

  unsigned long elapsed = now - previousMillis;

  int remaining = 0;
  if (elapsed < duration)
  {
    remaining = (duration - elapsed + 999) / 1000 - 1;
    if (remaining < 0)
      remaining = 0;
  }
  else
  {
    remaining = 0;
  }

  // ===== SERIAL: LUÔN IN (KỂ CẢ CHƯA BẤM NÚT) =====
  if (remaining != lastDisplayedTime)
  {
    Serial.print("[");
    Serial.print(ledName);
    Serial.print("] => seconds: ");
    Serial.println(remaining);

    lastDisplayedTime = remaining;

    // ===== DISPLAY: CHỈ HIỂN THỊ KHI BẤM NÚT =====
    if (systemRunning)
    {
      display.showNumberDec(remaining, false);
    }
  }

  // ===== HẾT GIỜ → ĐỔI ĐÈN =====
  if (elapsed >= duration)
  {
    if (state == 1)
      digitalWrite(RED_LED, LOW);
    if (state == 2)
      digitalWrite(GREEN_LED, LOW);
    if (state == 3)
      digitalWrite(YELLOW_LED, LOW);

    if (state == 1)
      state = 2;
    else if (state == 2)
      state = 3;
    else if (state == 3)
      state = 1;

    previousMillis = now;
    lastDisplayedTime = -1;

  }

  // ===== NHẤP NHÁY ĐÈN =====
  if (now - blinkMillis >= 500)
  {
    blinkMillis = now;
    ledBlink = !ledBlink;

    if (state == 1)
      digitalWrite(RED_LED, ledBlink);
    else if (state == 2)
      digitalWrite(GREEN_LED, ledBlink);
    else if (state == 3)
      digitalWrite(YELLOW_LED, ledBlink);
  }
}

// ====== LOOP ======
void loop()
{
  handleButton();
  checkLight();
  if (isNight){
    yellowBlink(millis());
  }else{
    trafficLight(millis());
  }
  
}
