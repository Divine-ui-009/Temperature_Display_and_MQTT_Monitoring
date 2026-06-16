/*
  Temperature Display and MQTT Monitoring - Part 1 (Final)
  Trade Code: SPE - Embedded Systems Software Integration

  This sketch auto-detects your LCD's I2C address on boot, so you don't
  need to guess 0x27 vs 0x3F. Watch the Serial Monitor at startup.

  Wiring:
    LCD (I2C backpack): GND->GND, VCC->5V, SDA->A4, SCL->A5
    DHT11 module:       VCC->5V, GND->GND, OUT->D2
*/

#include <Wire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// ----- USER CONFIG -----
String CANDIDATE_NAME = "STUDENT IRASUBIZA DIVINE";   // <-- change to your name
const unsigned long READ_INTERVAL_MS = 2000;
const unsigned long SCROLL_INTERVAL_MS = 350;
// ------------------------
#define hardcoded_vcc A2
#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C* lcd = nullptr;


uint8_t lcdAddress = 0;
bool lcdFound = false;

unsigned long lastRead = 0;
unsigned long lastScroll = 0;
int scrollPos = 0;
String scrollText;
bool needsScrolling = false;

uint8_t findLcdAddress() {
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      return addr;
    }
  }
  return 0;
}

void setup() {
  Serial.begin(9600);

  pinMode(hardcoded_vcc, OUTPUT);
  digitalWrite(hardcoded_vcc, HIGH);

  Wire.begin();
  dht.begin();

  Serial.println("Scanning I2C bus for LCD...");
  lcdAddress = findLcdAddress();

  if (lcdAddress == 0) {
    Serial.println("ERROR: No I2C device found. Check SDA->A4, SCL->A5, VCC->5V, GND->GND wiring.");
    lcdFound = false;
  } else {
    Serial.print("LCD found at address 0x");
    Serial.println(lcdAddress, HEX);
    lcd = new LiquidCrystal_I2C(lcdAddress, 16, 2);
    lcd->init();
    lcd->backlight();
    lcdFound = true;
  }

  needsScrolling = CANDIDATE_NAME.length() > 16;
  scrollText = needsScrolling ? (CANDIDATE_NAME + "   ") : CANDIDATE_NAME;

  if (lcdFound && !needsScrolling) {
    lcd->setCursor(0, 0);
    lcd->print(CANDIDATE_NAME);
  }

  if (lcdFound) {
    lcd->setCursor(0, 1);
    lcd->print("Starting...");
  }
}

void loop() {
  unsigned long now = millis();

  if (lcdFound && needsScrolling && now - lastScroll >= SCROLL_INTERVAL_MS) {
    lastScroll = now;
    lcd->setCursor(0, 0);
    for (int i = 0; i < 16; i++) {
      lcd->print(scrollText[(scrollPos + i) % scrollText.length()]);
    }
    scrollPos++;
    if (scrollPos >= (int)scrollText.length()) {
      scrollPos = 0;
    }
  }

  if (now - lastRead >= READ_INTERVAL_MS) {
    lastRead = now;
    float temp = dht.readTemperature();

    if (isnan(temp)) {
      Serial.println("ERR:SENSOR - check DHT11 wiring");
      if (lcdFound) {
        lcd->setCursor(0, 1);
        lcd->print("Sensor error    ");
      }
    } else {
      Serial.print("TEMP:");
      Serial.println(temp, 1);

      if (lcdFound) {
        lcd->setCursor(0, 1);
        lcd->print("Temp: ");
        lcd->print(temp, 1);
        lcd->print((char)223);
        lcd->print("C   ");
      }
    }
  }
}