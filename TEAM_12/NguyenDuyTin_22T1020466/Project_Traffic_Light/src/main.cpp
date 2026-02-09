#include <main.h>

#define PIN_LED_RED 23 // Chân nối LED đỏ
#define PIN_LED_GREEN 22 // Chân nối LED xanh
#define PIN_LED_YELLOW 21 // Chân nối LED vàng 

// Khai báo chân kết nối module TM1637
#define CLK 32
#define DIO 33

#define PIN_BUTTON_DISPLAY 27 // Chân nối nút bật/tắt hiển thị
#define PIN_LED_BLUE 26 //  Chân nối LED báo trạng thái
#define PIN_LDR 35 // Chân nối cảm biến ánh sáng LDR


TM1637Display display(CLK, DIO); // Tạo đối tượng hiển thị 7 đoạn
BUTTON btnBlue; // tạo đối tượng nút nhấn
LED ledBlue; // tạo đối tượng LED báo trạng thái
Traffic_Blink traffic; // tạo đối tượng điều khiển đèn giao thông
LDR ldrSensor; // tạo đối tượng cảm biến ánh sáng LDR

// Hàm xử lý nút nhấn bật/tắt hiển thị và cập nhật số đếm thời gian
void ProcessButtonPressed()
{
  static int prevSecond = -1; 
  static bool prevButton = false;
    // TRỜI TỐI → COI NHƯ NÚT ĐANG OFF
    if (traffic.isNightMode())
    {
        ledBlue.setStatus(false);
        display.clear();
        prevButton = false;   // reset trạng thái nút
        prevSecond = -1;
        return;
    }

  btnBlue.processPressed();
  bool pressed = btnBlue.isPressed();
  // Nếu trạng thái nút nhấn thay đổi
  if (prevButton != pressed)
  {
    if (pressed)
    {
      ledBlue.setStatus(true);
      printf("*** DISPLAY ON ***\n");
    }
    else
    {
      ledBlue.setStatus(false);
      display.clear();
      printf("*** DISPLAY OFF ***\n");
    }
    prevButton = pressed;
  }
//  Nếu nút nhấn đang được giữ thì cập nhật số đếm thời gian
  if (pressed)
  {
    int secondCount = traffic.getCount();
    if (prevSecond != secondCount)
    {
      prevSecond = secondCount;
      display.showNumberDec(secondCount);
    }
  }
}
// Hàm khởi tạo
void setup()
{
  printf("*** PROJECT TRAFFIC LIGHT ***\n");
  // ledYellow.setup(PIN_LED_YELLOW, "YELLOW");
  btnBlue.setup(PIN_BUTTON_DISPLAY);
  ledBlue.setup(PIN_LED_BLUE, "BLUE");
  display.setBrightness(0x0a); // Thiết lập độ sáng hiển thị
  display.clear();
  
  traffic.setup_Pin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);//  khởi tạo chân đèn giao thông
  traffic.setup_WaitTime(15, 5, 15);

  ldrSensor.setup(PIN_LDR, false); // ESP32 = 3.3V

}

void loop()
{
  // ledYellow.blink();
  ProcessButtonPressed(); 
  //traffic.blink(btnBlue.isPressed());// điều khiển đèn giao thông
  // Kiểm tra chế độ ngày/đêm  
  traffic.run(ldrSensor, btnBlue.isPressed());

}