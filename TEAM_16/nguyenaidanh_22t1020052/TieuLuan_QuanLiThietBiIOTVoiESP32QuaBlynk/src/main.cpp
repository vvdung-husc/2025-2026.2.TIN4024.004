#define BLYNK_TEMPLATE_ID "TMPL6zoKN_usl"
#define BLYNK_TEMPLATE_NAME "ESP32 TRAFFIC BLYNK"
#define BLYNK_AUTH_TOKEN "m6AxsNll-e3XFjJoTNUW0O551R1_qWSm"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// Định nghĩa chân cắm
#define DHTPIN 14
#define LDR_PIN 34
#define CLK 22
#define DIO 23
#define LED_RED 18
#define LED_YELLOW 5
#define LED_GREEN 17
#define LED_BLUE 12

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

DHT dht(DHTPIN, DHT22);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

float lastT = -1.0, lastH = -1.0;
int lastLDR = -1;
bool isDark = false;
bool isTrafficManualOff = false; 

// V1: Điều khiển LED Blue từ Blynk App
BLYNK_WRITE(V1) {
  int state = param.asInt();
  digitalWrite(LED_BLUE, state);
  
  // Bổ sung Terminal
  Serial.print("-> Blynk: LED XANH DUONG đang ");
  Serial.println(state == 1 ? "BAT [ON]" : "TAT [OFF]");
}

// V2: Điều khiển Bật/Tắt cụm đèn giao thông từ Blynk App
BLYNK_WRITE(V2) {
  int relayState = param.asInt();
  if (relayState == 0) {
    isTrafficManualOff = true;
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    display.clear();
    Blynk.virtualWrite(V0, 0); 
    
    // Bổ sung Terminal
    Serial.println("-> Blynk: DA TAT CUM DEN GIAO THONG!");
  } else {
    isTrafficManualOff = false;
    
    // Bổ sung Terminal
    Serial.println("-> Blynk: DA BAT CUM DEN GIAO THONG!");
  }
}

// Cập nhật cảm biến lên Blynk
void updateSensors(bool force = false) {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int ldrVal = analogRead(LDR_PIN);

  if (isnan(h) || isnan(t)) return;

  bool hasChanged = false;
  
  if (abs(t - lastT) >= 0.5) { 
    Blynk.virtualWrite(V5, t); 
    lastT = t; 
    hasChanged = true; 
  }
  
  if (abs(h - lastH) >= 1.0) { 
    Blynk.virtualWrite(V3, h); 
    lastH = h; 
    hasChanged = true; 
  }
  
  if (abs(ldrVal - lastLDR) >= 50) { 
    Blynk.virtualWrite(V4, ldrVal); 
    lastLDR = ldrVal; 
    hasChanged = true; 
    isDark = (ldrVal > 2000); 
  }

  if (hasChanged || force) {
    Serial.println("\n================================");
    Serial.println("   THÔNG SỐ THIẾT BỊ: ");
    Serial.print(" Nhiệt độ: "); Serial.print(t, 1); Serial.println(" *C");
    Serial.print(" Độ ẩm:    "); Serial.print(h, 1); Serial.println(" %");
    Serial.print(" Ánh sáng: "); Serial.println(ldrVal);
    Serial.println("================================");
  }
}

void blinkLight(int pin, int seconds) {
  for (int i = seconds; i >= 0; i--) {
    if (isTrafficManualOff || isDark) {
      digitalWrite(pin, LOW);
      return;
    }

    display.showNumberDec(i);
    Blynk.virtualWrite(V0, i); 

    digitalWrite(pin, HIGH); 
    delay(500);
    digitalWrite(pin, LOW);  
    delay(500);
    
    Blynk.run(); 
  }
}

void trafficLogic() {
  if (isTrafficManualOff) return; 

  if (!isDark) {
    blinkLight(LED_GREEN, 7);
    blinkLight(LED_YELLOW, 3);
    blinkLight(LED_RED, 10);
  } else {
    display.showNumberDec(88);
    Blynk.virtualWrite(V0, 0);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    
    digitalWrite(LED_YELLOW, HIGH); delay(500);
    digitalWrite(LED_YELLOW, LOW); delay(500);
    Blynk.run();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  display.setBrightness(0x0f);
  dht.begin();

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  
  Serial.println("\nWiFi Connected!");
  Blynk.config(auth);

  updateSensors(true); 
  timer.setInterval(2000L, [](){ updateSensors(false); });
}

void loop() {
  Blynk.run(); 
  timer.run();
  trafficLogic();
}