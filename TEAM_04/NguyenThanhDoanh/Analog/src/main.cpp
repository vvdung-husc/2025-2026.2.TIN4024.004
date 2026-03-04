#include <TM1637Display.h>
#include <HWCDC.h>
#include <HardwareSerial.h>

// Pins
#define CLK 15
#define DIO 2
#define RED_LED 25
#define YELLOW_LED 33
#define GREEN_LED 32
#define LDR_PIN 34

TM1637Display display(CLK, DIO);

const float GAMMA = 0.7;
const float RL10 = 50;

void setup()
{
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  display.setBrightness(0x0f);
}

float getLux()
{
  int analogValue = analogRead(LDR_PIN);
  float voltage = analogValue / 4095. * 3.3;
  float resistance = 2000 * voltage / (1 - voltage / 3.3);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
  return lux;
}

// Hàm bổ trợ để thực hiện việc nhấp nháy đèn trong khi đếm ngược
void blinkAndCountdown(int seconds, int ledPin)
{
  for (int i = seconds; i > 0; i--)
  {
    display.showNumberDec(i);

    // Kiểm tra ánh sáng: Nếu bất ngờ tối thì thoát ngay lập tức
    if (getLux() < 10)
      return;

    // Hiệu ứng nhấp nháy trong 1 giây: 500ms sáng, 500ms tắt
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}

void nightMode()
{
  // Tắt các đèn không liên quan
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  // Nhấp nháy đèn vàng liên tục (Chu kỳ 1s)
  digitalWrite(YELLOW_LED, HIGH);
  delay(500);
  digitalWrite(YELLOW_LED, LOW);
  delay(500);
  display.showNumberDec(0);
}

void dayMode()
{
  // Thứ tự: Đỏ (5s) -> Xanh (5s) -> Vàng (3s)

  Serial.println("Day Mode: Red Blinking");
  blinkAndCountdown(5, RED_LED);
  if (getLux() < 10)
    return;

  Serial.println("Day Mode: Green Blinking");
  blinkAndCountdown(5, GREEN_LED);
  if (getLux() < 10)
    return;

  Serial.println("Day Mode: Yellow Blinking");
  blinkAndCountdown(3, YELLOW_LED);
}

void loop()
{
  float currentLux = getLux();
  Serial.print("Lux: ");
  Serial.println(currentLux);

  if (currentLux < 10)
  {
    nightMode();
  }
  else
  {
    dayMode();
  }
}