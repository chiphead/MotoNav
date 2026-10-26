#pragma once

// CST820 touch adapter for Waveshare ESP32-S3-Touch-LCD-2.1B.
// The board routes touch reset through TCA9554 EXIO2; do not assign a GPIO
// reset pin to TouchDrvCST816. The CST820 is handled by SensorLib's CST816
// family driver (its supported chip IDs include CST820/B7).

#include <Arduino.h>
#include <Wire.h>
#include "../../libraries/SensorLib/src/touch/TouchDrvCST816.hpp"
#include "pin_config.h"

namespace Board21B {

class Touch {
 public:
  bool begin() {
    // initExpander() must already have released EXIO2 before this call.
    driver_.setPins(-1, TP_INT);
    driver_.setMirrorXY(BOARD_TOUCH_FLIP_X != 0, BOARD_TOUCH_FLIP_Y != 0);
    driver_.setMaxCoordinates(LCD_WIDTH - 1, LCD_HEIGHT - 1);
    ready_ = driver_.begin(Wire, 0x15, IIC_SDA, IIC_SCL);
    return ready_;
  }

  bool available() const { return ready_; }

  // Returns true when at least one touch point is present. Coordinates are
  // already transformed by SensorLib's configured mirror/scaling settings.
  bool read(uint16_t &x, uint16_t &y) {
    if (!ready_) return false;
    const TouchPoints &points = driver_.getTouchPoints();
    if (!points.hasPoints()) return false;
    const TouchPoint &point = points.getPoint(0);
    x = static_cast<uint16_t>(constrain(point.x, 0, LCD_WIDTH - 1));
    y = static_cast<uint16_t>(constrain(point.y, 0, LCD_HEIGHT - 1));
    return true;
  }

 private:
  TouchDrvCST816 driver_;
  bool ready_ = false;
};

}  // namespace Board21B
