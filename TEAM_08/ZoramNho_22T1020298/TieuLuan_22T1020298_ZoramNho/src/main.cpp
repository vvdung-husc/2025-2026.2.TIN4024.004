#define BLYNK_TEMPLATE_ID "TMPL6hVzLt9rU"
#define BLYNK_TEMPLATE_NAME "BLYNK DIEU KHIEN LED"
#define BLYNK_AUTH_TOKEN "2voxQDRwdj4-FR9HUzrVdGdZYbaYLnvd"

// 2. Thêm các thư viện cần thiết
#include <Arduino.h>  // Bắt buộc khi dùng VS Code
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// 3. Thông số mạng WiFi
char ssid[] = "Wokwi-GUEST"; // Thay đổi nếu chạy trên mạch thật
char pass[] = "";            // Thay đổi nếu chạy trên mạch thật

// 4. Cấu hình chân Pin
#define LED_PIN 23 

// ===========================================================
// Hàm nhận dữ liệu từ App Blynk (Virtual Pin V0)
// ===========================================================
BLYNK_WRITE(V0) {
    int value = param.asInt(); // Lấy giá trị 0 hoặc 1 từ nút nhấn

    if (value == 1) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println(">>> TRẠNG THÁI: BẬT ĐÈN");
    } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println(">>> TRẠNG THÁI: TẮT ĐÈN");
    }
}

// ===========================================================
// Hàm thiết lập ban đầu (Setup)
// ===========================================================
void setup() {
    // Khởi tạo Serial để debug
    Serial.begin(115200);
    delay(100);

    // Cấu hình chân LED/Relay
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Mặc định tắt khi mới khởi động

    Serial.println("--------------------------------------");
    Serial.println("HE THONG DANG KHOI DONG...");
    
    // Kết nối tới Blynk Cloud
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    Serial.println("KET NOI THANH CONG!");
    Serial.println("--------------------------------------");
}

// ===========================================================
// Vòng lặp chính (Loop)
// ===========================================================
void loop() {
    Blynk.run(); // Duy trì kết nối và xử lý lệnh từ Blynk Cloud
}