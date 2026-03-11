#pragma once

// --- BLYNK CONFIGURATION ---
// Khai báo Blynk phải đặt lên trên cùng
#define BLYNK_TEMPLATE_ID "TMPL6MmOgzaXh"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "kDc6eUsp8Gz4F_BA5JCIo7TPo_ZZOioM"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "ultils.h"