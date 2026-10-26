#pragma once

// Board bring-up reference for Waveshare ESP32-S3-Touch-LCD-2.1B.
// Hardware facts and vendor init table are adapted from the Waveshare 2.1-inch
// board example / ESP32_Display_Panel board definition. See BOARD_NOTES.md.
// This header is intentionally not included by MotoNav.ino yet: wire it into
// the firmware only after the project's Arduino_GFX version and render path
// have been compile-tested together.

#include <Arduino.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"

namespace Board21B {

constexpr uint8_t TCA_REG_OUTPUT = 0x01;
constexpr uint8_t TCA_REG_POLARITY = 0x02;
constexpr uint8_t TCA_REG_CONFIG = 0x03;
constexpr uint8_t EXIO_LCD_RST_MASK = 1u << (BOARD_EXIO_LCD_RESET - 1);
constexpr uint8_t EXIO_TP_RST_MASK = 1u << (BOARD_EXIO_TOUCH_RESET - 1);
constexpr uint8_t EXIO_LCD_CS_MASK = 1u << (BOARD_EXIO_LCD_CS - 1);
constexpr uint8_t EXIO_SD_CS_MASK = 1u << 3; // EXIO4: SD card shares GPIO1/GPIO2
constexpr uint8_t EXPANDER_OUTPUT_MASK =
    EXIO_LCD_RST_MASK | EXIO_TP_RST_MASK | EXIO_LCD_CS_MASK | EXIO_SD_CS_MASK;
inline uint8_t expanderOutputState = 0;

inline bool writeTcaRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(TCA9554_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

// Establish safe reset/CS levels before enabling the LCD. Unused expander
// pins remain inputs; this avoids accidentally driving the buzzer (EXIO8).
inline bool initExpander() {
  Wire.begin(IIC_SDA, IIC_SCL, 100000);
  delay(20);
  // Start with both controllers held in reset; keep both chip-selects high.
  expanderOutputState = EXIO_LCD_CS_MASK | EXIO_SD_CS_MASK;
  if (!writeTcaRegister(TCA_REG_OUTPUT, expanderOutputState)) return false;
  // Force non-inverted expander inputs/outputs rather than relying on the
  // chip's power-on default or a previous sketch's register state.
  if (!writeTcaRegister(TCA_REG_POLARITY, 0x00)) return false;
  if (!writeTcaRegister(TCA_REG_CONFIG, static_cast<uint8_t>(~EXPANDER_OUTPUT_MASK))) return false;
  delay(10);
  // Release LCD reset, then release touch reset after its own low pulse.
  expanderOutputState |= EXIO_LCD_RST_MASK;
  if (!writeTcaRegister(TCA_REG_OUTPUT, expanderOutputState)) return false;
  delay(100);
  expanderOutputState |= EXIO_TP_RST_MASK;
  if (!writeTcaRegister(TCA_REG_OUTPUT, expanderOutputState)) return false;
  delay(50);
  return true;
}

// Arduino_GFX's generic XCA9554SWSPI assumes the whole serial bus is on the
// expander. On this board only CS is on TCA9554; SCL/SDA are GPIO2/GPIO1.
class ExpanderCsSWSPI final : public Arduino_SWSPI {
 public:
  ExpanderCsSWSPI()
      : Arduino_SWSPI(GFX_NOT_DEFINED, GFX_NOT_DEFINED, LCD_SCLK, LCD_SDA) {}

  void beginWrite() override {
    expanderOutputState &= static_cast<uint8_t>(~EXIO_LCD_CS_MASK);
    writeTcaRegister(TCA_REG_OUTPUT, expanderOutputState);
    Arduino_SWSPI::beginWrite();
  }

  void endWrite() override {
    Arduino_SWSPI::endWrite();
    expanderOutputState |= EXIO_LCD_CS_MASK;
    writeTcaRegister(TCA_REG_OUTPUT, expanderOutputState);
  }
};

// ST7701S vendor initialization sequence for the Waveshare 2.1-inch 480x480
// round panel. It is kept here separately from the 1.85B ST77916 command table.
inline const uint8_t st7701Init[] = {
    BEGIN_WRITE,
    WRITE_COMMAND_8, 0xFF, WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x10,
    WRITE_C8_D16, 0xC0, 0x3B, 0x00,
    WRITE_C8_D16, 0xC1, 0x0B, 0x02,
    WRITE_C8_D16, 0xC2, 0x07, 0x02,
    WRITE_C8_D8, 0xCC, 0x10,
    WRITE_C8_D8, 0xCD, 0x08,
    WRITE_COMMAND_8, 0xB0, WRITE_BYTES, 16,
    0x00, 0x11, 0x16, 0x0E, 0x11, 0x06, 0x05, 0x09, 0x08, 0x21, 0x06, 0x13, 0x10, 0x29, 0x31, 0x18,
    WRITE_COMMAND_8, 0xB1, WRITE_BYTES, 16,
    0x00, 0x11, 0x16, 0x0E, 0x11, 0x07, 0x05, 0x09, 0x09, 0x21, 0x05, 0x13, 0x11, 0x2A, 0x31, 0x18,
    WRITE_COMMAND_8, 0xFF, WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x11,
    WRITE_C8_D8, 0xB0, 0x6D,
    WRITE_C8_D8, 0xB1, 0x37,
    WRITE_C8_D8, 0xB2, 0x81,
    WRITE_C8_D8, 0xB3, 0x80,
    WRITE_C8_D8, 0xB5, 0x43,
    WRITE_C8_D8, 0xB7, 0x85,
    WRITE_C8_D8, 0xB8, 0x20,
    WRITE_C8_D8, 0xC1, 0x78,
    WRITE_C8_D8, 0xC2, 0x78,
    WRITE_C8_D8, 0xD0, 0x88,
    WRITE_COMMAND_8, 0xE0, WRITE_BYTES, 3, 0x00, 0x00, 0x02,
    WRITE_COMMAND_8, 0xE1, WRITE_BYTES, 11, 0x03, 0xA0, 0x00, 0x00, 0x04, 0xA0, 0x00, 0x00, 0x00, 0x20, 0x20,
    WRITE_COMMAND_8, 0xE2, WRITE_BYTES, 13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    WRITE_COMMAND_8, 0xE3, WRITE_BYTES, 4, 0x00, 0x00, 0x11, 0x00,
    WRITE_C8_D16, 0xE4, 0x22, 0x00,
    WRITE_COMMAND_8, 0xE5, WRITE_BYTES, 16, 0x05, 0xEC, 0xA0, 0xA0, 0x07, 0xEE, 0xA0, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    WRITE_COMMAND_8, 0xE6, WRITE_BYTES, 4, 0x00, 0x00, 0x11, 0x00,
    WRITE_C8_D16, 0xE7, 0x22, 0x00,
    WRITE_COMMAND_8, 0xE8, WRITE_BYTES, 16, 0x06, 0xED, 0xA0, 0xA0, 0x08, 0xEF, 0xA0, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    WRITE_COMMAND_8, 0xEB, WRITE_BYTES, 7, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00,
    WRITE_COMMAND_8, 0xED, WRITE_BYTES, 16, 0xFF, 0xFF, 0xFF, 0xBA, 0x0A, 0xBF, 0x45, 0xFF, 0xFF, 0x54, 0xFB, 0xA0, 0xAB, 0xFF, 0xFF, 0xFF,
    WRITE_COMMAND_8, 0xEF, WRITE_BYTES, 6, 0x10, 0x0D, 0x04, 0x08, 0x3F, 0x1F,
    WRITE_COMMAND_8, 0xFF, WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x13,
    WRITE_C8_D8, 0xEF, 0x08,
    WRITE_COMMAND_8, 0xFF, WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x00,
    WRITE_C8_D8, 0x36, 0x00,
    WRITE_C8_D8, 0x3A, 0x66,
    WRITE_COMMAND_8, 0x11,
    DELAY, 240, DELAY, 240,
    WRITE_COMMAND_8, 0x20,
    DELAY, 120,
    WRITE_COMMAND_8, 0x29,
    END_WRITE
};

inline Arduino_ESP32RGBPanel* createRgbPanel() {
  constexpr int32_t pixelClockHz = 16000000;
  constexpr size_t bounceBufferPixels = 480 * 10;
  return new Arduino_ESP32RGBPanel(
      LCD_DE, LCD_VSYNC, LCD_HSYNC, LCD_PCLK,
      LCD_R1, LCD_R2, LCD_R3, LCD_R4, LCD_R5,
      LCD_G0, LCD_G1, LCD_G2, LCD_G3, LCD_G4, LCD_G5,
      LCD_B1, LCD_B2, LCD_B3, LCD_B4, LCD_B5,
      0, 50, 8, 10,  // HSYNC active-low, front porch, pulse width, back porch
      0, 8, 3, 8,    // VSYNC active-low, front porch, pulse width, back porch
      0, pixelClockHz, false, 0, 0, bounceBufferPixels);
}

inline Arduino_RGB_Display* createDisplay(Arduino_DataBus* bus,
                                          Arduino_ESP32RGBPanel* panel) {
  return new Arduino_RGB_Display(LCD_WIDTH, LCD_HEIGHT, panel, 0, true, bus,
                                 GFX_NOT_DEFINED, st7701Init, sizeof(st7701Init));
}

} // namespace Board21B
