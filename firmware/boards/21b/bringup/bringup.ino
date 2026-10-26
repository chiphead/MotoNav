// Minimal hardware bring-up sketch for Waveshare ESP32-S3-Touch-LCD-2.1B.
// Copy this sketch into a folder named "bringup" and compile with the same
// Arduino-ESP32 core and GFX/SensorLib libraries as MotoNav.
// This validates the board-specific display/touch path independently of MotoNav.
// It is NOT the MotoNav firmware and has not yet been compiled on hardware.

#include <Arduino.h>
#include <Wire.h>
#include "../MotoNav/Board21BDisplay.h"
#include "../MotoNav/Board21BTouch.h"

Arduino_ESP32RGBPanel *rgbPanel = nullptr;
Arduino_RGB_Display *display = nullptr;
Board21B::Touch touch;

static void showColorBars() {
  display->fillScreen(0x0000);
  const uint16_t colors[] = {0xF800, 0x07E0, 0x001F, 0xFFFF, 0xFFE0, 0xF81F, 0x07FF};
  const int n = sizeof(colors) / sizeof(colors[0]);
  const int band = LCD_HEIGHT / n;
  for (int i = 0; i < n; ++i) {
    display->fillRect(0, i * band, LCD_WIDTH, band, colors[i]);
  }
  display->drawCircle(LCD_WIDTH / 2, LCD_HEIGHT / 2, 150, 0x0000);
  display->drawCircle(LCD_WIDTH / 2, LCD_HEIGHT / 2, 151, 0xFFFF);
  display->setTextColor(0x0000);
  display->setTextSize(2);
  display->setCursor(150, LCD_HEIGHT / 2 - 8);
  display->print("2.1B RGB");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nMotoNav 2.1B display/touch bring-up");

  if (!Board21B::initExpander()) {
    Serial.println("ERROR: TCA9554 not responding; check I2C pins/address.");
    while (true) delay(1000);
  }

  rgbPanel = Board21B::createRgbPanel();
  Arduino_DataBus *bus = new Board21B::ExpanderCsSWSPI();
  display = Board21B::createDisplay(bus, rgbPanel);
  if (!display->begin()) {
    Serial.println("ERROR: RGB display initialization/framebuffer allocation failed.");
    while (true) delay(1000);
  }

  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  showColorBars();

  if (touch.begin()) {
    Serial.println("Touch: initialized (CST820 family driver).");
  } else {
    Serial.println("WARNING: touch initialization failed; display test continues.");
  }
}

void loop() {
  static uint32_t lastLog = 0;
  uint16_t x = 0, y = 0;
  if (touch.read(x, y)) {
    Serial.printf("Touch x=%u y=%u\n", x, y);
    delay(80);
  }
  if (millis() - lastLog >= 2000) {
    lastLog = millis();
    Serial.printf("uptime=%lu ms, touch=%s\n",
                  static_cast<unsigned long>(millis()),
                  touch.available() ? "ready" : "not-ready");
  }
  delay(10);
}
