#include "ultils.h"

// ----- Functions -----
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

// ----- LED -----
LED::LED() {
  _pin = -1;
  _state = false;
  _previousMillis = 0;
}

void LED::setup(int pin) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
  off();
}

void LED::on() {
  _state = true;
  digitalWrite(_pin, HIGH);
}

void LED::off() {
  _state = false;
  digitalWrite(_pin, LOW);
}

void LED::toggle() {
  _state = !_state;
  digitalWrite(_pin, _state ? HIGH : LOW);
}

void LED::blink(unsigned long interval) {
  if (!IsReady(_previousMillis, interval)) return;
  toggle();
}

// ----- Trafic_Blink -----
Trafic_Blink::Trafic_Blink() {
  _ledStatus = false;
  _previousMillis = 0;
  _idxLED = 0;
  _waitTime[0] = _waitTime[1] = _waitTime[2] = 1000;
  _LEDs[0] = _LEDs[1] = _LEDs[2] = -1;
}

void Trafic_Blink::setupPin(int pinRed, int pinYellow, int pinGreen) {
  _LEDs[0] = pinGreen;
  _LEDs[1] = pinYellow;
  _LEDs[2] = pinRed;

  pinMode(pinRed, OUTPUT);
  pinMode(pinYellow, OUTPUT);
  pinMode(pinGreen, OUTPUT);

  digitalWrite(pinRed, LOW);
  digitalWrite(pinYellow, LOW);
  digitalWrite(pinGreen, LOW);

  _idxLED = 0;
}

void Trafic_Blink::setupWaitTime(uint32_t redWait, uint32_t yellowWait, uint32_t greenWait) {
  _waitTime[0] = greenWait * 1000UL;
  _waitTime[1] = yellowWait * 1000UL;
  _waitTime[2] = redWait * 1000UL;
}

void Trafic_Blink::run() {
  static uint32_t count = 0;

  // tick mỗi 500ms giống thầy
  if (!IsReady(_previousMillis, 500)) return;

  // lần đầu vào
  if (count == 0) {
    count = _waitTime[_idxLED];

    // bật LED hiện tại, tắt 2 cái còn lại
    for (int i = 0; i < 3; i++) {
      digitalWrite(_LEDs[i], (i == _idxLED) ? HIGH : LOW);
    }

    _ledStatus = true; // đang ON
    return;
  }

  // Blink LED đang chạy (nhấp nháy 500ms)
  _ledStatus = !_ledStatus;
  digitalWrite(_LEDs[_idxLED], _ledStatus ? HIGH : LOW);

  // giảm thời gian
  if (count >= 500) count -= 500;

  // hết thời gian -> chuyển sang LED tiếp theo
  if (count == 0) {
    _idxLED = (_idxLED + 1) % 3;
  }
}
