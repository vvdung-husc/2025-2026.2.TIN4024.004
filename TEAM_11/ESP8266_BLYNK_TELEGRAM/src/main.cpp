	/*
	THÔNG TIN NHÓM 11
	1. Nguyễn Đình Hoàng
	2.
	3. ...
	*/
#include <Arduino.h>


int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}