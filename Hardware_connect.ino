// 🔹 1. INCLUDE & WIFI & API
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PCF8574.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===== WIFI =====
const char *ssid = "DUC CO";
const char *password = "ducco2711";

// ===== BACKEND =====
const String BASE_URL = "https://smart-parking-server-b8oa.onrender.com";

// SLOT ID BE
const char *SLOT_IDS[3] = {
    "695c82d7936586d8a602ae70",
    "695c82d7936586d8a602ae71",
    "695c82d7936586d8a602ae72"};

WebServer server(80);

// 🔹 2. LCD – PCF – SERVO – PIN
#define LCD_ADDR 0x27
#define PCF_ADDR 0x20
#define PCF_LED_ADDR 0x21

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
PCF8574 pcf(PCF_ADDR);
PCF8574 pcfLeds(PCF_LED_ADDR);

Servo servoIn, servoOut, servoRoof;

// ===== PIN =====
#define TRIG_IN 33
#define ECHO_IN 32
#define SERVO_IN 25

#define TRIG_OUT 27
#define ECHO_OUT 14
#define SERVO_OUT 13

#define TRIG_P1 4
#define ECHO_P1 16
#define TRIG_P2 5
#define ECHO_P2 17
#define TRIG_P3 18
#define ECHO_P3 19

#define FIRE_PIN 26
#define GAS_PIN 34
#define RAIN_PIN 35
#define SERVO_ROOF_PIN 23

#define LIGHT_SENSOR_PIN 36
#define LED_PIN 2
#define BUZZER_PIN 12

// 🔹 3. THAM SỐ – SLOT – XE
#define MAX_CARS 3
#define DIST_THRESHOLD 10
#define SLOT_THRESHOLD 15
#define PASSWORD_TIMEOUT 5000
#define GATE_CLOSE_DELAY 4000
#define PRICE_PER_HOUR 20000

String carID[MAX_CARS];
unsigned long carTimeIn[MAX_CARS];

bool slot[3];
bool lastSlot[3];

bool fireMode = false;
bool gasMode = false;
bool isRaining = false;
bool isDark = false;

unsigned long lastSlotUpdate = 0;

// 🔹 4. HÀM GỌI API CHUNG
void postJSON(String url, String body)
{
    if (WiFi.status() != WL_CONNECTED)
        return;

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    int code = http.POST(body);
    Serial.printf("[API] %s -> %d\n", url.c_str(), code);

    http.end();
}

// 🔹 5. API CỤ THỂ
// 🔸 Check-in
void apiCheckIn(String phone)
{
    String body = "{\"phone\":\"" + phone + "\",\"password\":\"123456\"}";
    postJSON(BASE_URL + "/users/check-in", body);
}

// 🔸 Check-out
void apiCheckOut(String phone)
{
    String body = "{\"phone\":\"" + phone + "\",\"password\":\"123456\"}";
    postJSON(BASE_URL + "/users/check-out", body);
}

// 🔸 Cháy / Gas
void apiFire() { postJSON(BASE_URL + "/users/fire", "{}"); }
void apiGas() { postJSON(BASE_URL + "/users/gas", "{}"); }

// 🔸 Update Slot
void apiUpdateSlot(int i, bool occupied)
{
    String body = "{";
    body += "\"id\":\"" + String(SLOT_IDS[i]) + "\",";
    body += "\"name\":\"Slot A" + String(i + 1) + "\",";
    body += "\"status\":\"" + String(occupied ? "Occupied" : "Available") + "\"}";

    postJSON(BASE_URL + "/parking-lots/" + String(SLOT_IDS[i]), body);
}

// 🔹 6. KHOẢNG CÁCH & SLOT
long getDistance(int t, int e)
{
    digitalWrite(t, LOW);
    delayMicroseconds(2);
    digitalWrite(t, HIGH);
    delayMicroseconds(10);
    digitalWrite(t, LOW);
    long d = pulseIn(e, HIGH, 25000);
    return d == 0 ? 999 : d * 0.034 / 2;
}

void updateSlots()
{
    slot[0] = getDistance(TRIG_P1, ECHO_P1) < SLOT_THRESHOLD;
    slot[1] = getDistance(TRIG_P2, ECHO_P2) < SLOT_THRESHOLD;
    slot[2] = getDistance(TRIG_P3, ECHO_P3) < SLOT_THRESHOLD;

    for (int i = 0; i < 3; i++)
    {
        if (slot[i] != lastSlot[i])
        {
            apiUpdateSlot(i, slot[i]);
            lastSlot[i] = slot[i];
        }
    }
}

// 🔹 7. SETUP
void setup()
{
    Serial.begin(115200);
    Wire.begin();

    lcd.init();
    lcd.backlight();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
        delay(500);

    pinMode(FIRE_PIN, INPUT);
    pinMode(GAS_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    servoIn.attach(SERVO_IN);
    servoOut.attach(SERVO_OUT);
    servoRoof.attach(SERVO_ROOF_PIN);

    memcpy(lastSlot, slot, sizeof(slot));
}

// 🔹 8. LOOP (CORE LOGIC)
void loop()
{
    updateSlots();

    // FIRE
    if (digitalRead(FIRE_PIN) && !fireMode)
    {
        fireMode = true;
        apiFire();
        servoIn.write(90);
        servoOut.write(90);
    }
    if (!digitalRead(FIRE_PIN))
        fireMode = false;

    // GAS
    if (digitalRead(GAS_PIN) && !gasMode)
    {
        gasMode = true;
        apiGas();
        servoIn.write(90);
        servoOut.write(90);
    }
    if (!digitalRead(GAS_PIN))
        gasMode = false;

    delay(300);
}