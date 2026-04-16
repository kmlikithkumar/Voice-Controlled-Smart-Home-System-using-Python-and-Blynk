// ── BLYNK CONFIG ─────────────────────────────
#define BLYNK_TEMPLATE_ID "TMPL3cLnYKI_X"
#define BLYNK_TEMPLATE_NAME "Smart Home Lighting"
#define BLYNK_AUTH_TOKEN "02h7LBP6Z0R-UhLK5G4s2W-OxmOaNwY-"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ── WiFi ─────────────────────────────────────
char ssid[] = "shahid";
char pass[] = "shahid123";

// ── Pins ─────────────────────────────────────
#define PIR_PIN   13
#define RELAY1    26
#define DHT_PIN   14
#define ACS_PIN   35
#define LED_PIN   2

// ── Objects ──────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT11);
BlynkTimer timer;

// ── Variables ────────────────────────────────
bool showMotion = true;
float temp = 0;
float current = 0;

// ── BLYNK CONTROL (VOICE + APP) ─────────────

// Relay (Light) Control → V1
BLYNK_WRITE(V1) {
  int value = param.asInt();

  if (value == 1) {
    digitalWrite(RELAY1, LOW);   // ON
  } else {
    digitalWrite(RELAY1, HIGH);  // OFF
  }
}

// LED Control → V2
BLYNK_WRITE(V2) {
  int value = param.asInt();

  if (value == 1) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

// ── Motion Detection ─────────────────────────
void checkMotion() {
  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH) {
    digitalWrite(RELAY1, LOW);
    Blynk.virtualWrite(V1, 1);
    Serial.println("Motion → Light ON");
  } else {
    digitalWrite(RELAY1, HIGH);
    Blynk.virtualWrite(V1, 0);
    Serial.println("No Motion → Light OFF");
  }
}

// ── Read Sensors ─────────────────────────────
void readSensors() {
  temp = dht.readTemperature();

  int raw = analogRead(ACS_PIN);
  float volts = (raw / 4095.0) * 3.3;
  current = (volts - 1.65) / 0.185;

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C | Current: ");
  Serial.println(current);
}

// ── LCD Display ──────────────────────────────
void updateLCD() {

  lcd.clear();

  if (showMotion) {
    int motion = digitalRead(PIR_PIN);

    lcd.setCursor(0, 0);
    lcd.print("Motion:");

    if (motion == HIGH) {
      lcd.print("Detected");
    } else {
      lcd.print("None   ");
    }

    lcd.setCursor(0, 1);
    lcd.print("Light:");
    lcd.print(digitalRead(RELAY1) == LOW ? "ON " : "OFF");

  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp:");
    lcd.print(temp);
    lcd.print("C   ");

    lcd.setCursor(0, 1);
    lcd.print("Curr:");
    lcd.print(abs(current));
    lcd.print("A   ");
  }

  showMotion = !showMotion;
}

// ── Setup ────────────────────────────────────
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(RELAY1, HIGH);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1500);
  lcd.clear();

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  // Timers
  timer.setInterval(300L, checkMotion);
  timer.setInterval(2000L, readSensors);
  timer.setInterval(5000L, updateLCD);
}

// ── Loop ─────────────────────────────────────
void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    delay(2000);
    return;
  }

  if (!Blynk.connected()) {
    Blynk.connect();
  }

  Blynk.run();
  timer.run();
}