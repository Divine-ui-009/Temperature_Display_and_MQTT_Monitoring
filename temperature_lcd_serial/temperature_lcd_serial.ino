/*
  Temperature Display and MQTT Monitoring - Part 1 (Final, analog sensor version)
  Trade Code: SPE - Embedded Systems Software Integration

  Uses an analog temperature sensor (e.g. LM35-style, 3-pin) on A0 instead of
  a DHT11. LCD auto-detects its I2C address on boot.

  Wiring:
    LCD (I2C backpack): GND->GND, VCC->5V, SDA->A4, SCL->A5
    Temp sensor:         VCC->5V (or A2 if 5V pin is full), GND->GND, OUT->A0
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ----- USER CONFIG -----
String CANDIDATE_NAME = "STUDENT DIVINE IRASUBIZA";   // <-- change to your name
const int TEMP_PIN = A0;
const unsigned long READ_INTERVAL_MS = 2000;
const unsigned long SCROLL_INTERVAL_MS = 350;
// ------------------------

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

float readTemp() {
  int raw = analogRead(TEMP_PIN);
  float voltage = raw * 5.0 / 1023.0;
  float tempC = voltage * 100.0; // LM35: 10mV per degree C
  return tempC;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

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
    float temp = readTemp();

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