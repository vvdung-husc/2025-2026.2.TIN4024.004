/*
THÔNG TIN NHÓM X
1. Nguyễn Văn Bình - Telegram : binhnv
2. Lê Nguyễn Hương Nguyên - Telegram : nguyen2019
3. ...
4. Nguyễn Bá Quý Đạt - Telegram : nguyenbaquydat
*/

#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup()
{
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop()
{
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}