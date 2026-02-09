#include <Arduino.h>

#define PIN_LED_RED 23 // Chân nối LED đỏ
#define PIN_LED_GREEN 22 // Chân nối LED xanh
#define PIN_LED_YELLOW 21 // Chân nối LED vàng 

// Hàm tạo bộ đếm thời gian (non-blocking)
bool IsReady(unsigned long &uTimer, uint32_t millisecond){
  if (millis() - uTimer < millisecond) return false; // Chưa đủ thời gian thì thoát
  uTimer = millis(); // Cập nhật lại thời gian
  return true; 
}

void ALLOFF(){ // Tắt tất cả LED
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
}

void setup() {
  printf("Setup LED Blink\n");
  pinMode(PIN_LED_RED, OUTPUT); // Cấu hình chân nối LED đỏ là OUTPUT
  pinMode(PIN_LED_GREEN, OUTPUT); // Cấu hình chân nối LED xanh là OUTPUT
  pinMode(PIN_LED_YELLOW, OUTPUT); // Cấu hình chân nối LED vàng là OUTPUT
}
// Hàm vòng lặp chính
void loop() {
  //static unsigned long uTimerLED = 0; // Biến lưu thời gian cho LED
  static unsigned long timerBlink =0; // Thời gian nhấp nháy
  static unsigned long timerPhase =0; // Thời gian pha
  static bool lesStatus = false; // Trạng thái hiện tại của LED
  static int phase =0; // Pha hiện tại

  ALLOFF(); // Tắt tất cả LED trước khi cập nhật trạng thái mới

   if (IsReady(timerBlink, 500)){ // Kiểm tra đã đủ 500ms chưa
    lesStatus = !lesStatus; // Đảo trạng thái LED
    
    //printf("LES IS [%s]\n", lesStatus ? "ON" : "OFF"); // In trạng thái LED ra console
    //digitalWrite(PIN_LED_RED, lesStatus); // Cập nhật trạng thái LED
  }
  

  if (phase == 0){ // Pha 0: LED đỏ
    digitalWrite(PIN_LED_RED, lesStatus); // Cập nhật trạng thái LED đỏ
    if (IsReady(timerPhase, 5000)) { // Sau 5 giây chuyển pha
      printf ("LED [RED] ON => 5 seconds\n");
      phase = 1; 
    }
  }
  else if (phase == 1){ // Pha 1: LED xanh
    digitalWrite(PIN_LED_GREEN, lesStatus); // Cập nhật trạng thái LED xanh
    if (IsReady(timerPhase, 7000)) { // Sau 7 giây chuyển pha
      printf ("LED [GREEN] ON => 7 seconds\n");
      phase = 2; 
     } 
  }
  else if (phase == 2){ // Pha 2: LED vàng
    digitalWrite(PIN_LED_YELLOW, lesStatus); // Cập nhật trạng thái LED vàng
    if (IsReady(timerPhase, 3000)) { // Sau 3 giây chuyển pha
      printf ("LED [YELLOW] ON => 3 seconds\n"); 
      phase = 0; 
     } 
  }
}


