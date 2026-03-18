/*
Thông tin nhóm 6
1. Nguyễn Hữu Trình-22T1020481


*/



#include "main.h"

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
  bot.begin(BOT_TOKEN);

}
