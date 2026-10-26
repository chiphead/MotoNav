# 2.1B Port — Source Code Review

Review scope: the current branch's 2.1B board adapter, isolated bring-up sketch, vendored Arduino_GFX/SensorLib APIs and PowerShell build entry point. This is a source review only: no Arduino CLI compile, flash, or hardware run was performed.

## Summary

The board adapter is still isolated from the MotoNav application. The GPIO mapping and Arduino_GFX constructor argument ordering are consistent with the local library declarations and the known Waveshare board pin map. The RGB display uses one RGB-panel framebuffer in PSRAM; the display class obtains a pointer to that framebuffer rather than allocating a second full framebuffer.

No claim of build success is made. The remaining risks below are the items to resolve before integrating the large MotoNav sketch.

## Findings

### R1 — High: no compile/test evidence yet

**Evidence:** `firmware/boards/21b/bringup/bringup.ino` and `Board21BDisplay.h` have not been compiled by this review environment. C++ API agreement by inspection does not prove that the Arduino CLI build will resolve all library dependencies or preprocess the sketch as expected.

**Action:** Run `firmware/build-21b-bringup.ps1` on the target Windows Arduino CLI setup; retain the complete first compiler error if it fails. Do not integrate the 9k-line MotoNav sketch until this passes.

### R2 — High: panel initialization errors are not surfaced reliably

**Evidence:** The vendored `Arduino_RGB_Display::begin()` calls `_rgbpanel->begin(speed)` without checking a returned status, then treats a non-null framebuffer as success. The panel implementation uses `ESP_ERROR_CHECK` internally for several failures, which may abort rather than return a clean error. The bring-up sketch currently reports a generic display initialization failure only if `display->begin()` returns false.

**Action:** During bring-up, capture serial logs and reset reasons. If initialization aborts, instrument the vendored panel driver or use the ESP-IDF error log to identify the failing allocation/panel operation. Avoid describing the current boolean check as a complete health check.

### R3 — Medium: expander chip-select I2C failures are ignored during display writes

**Evidence:** `Board21B::ExpanderCsSWSPI::beginWrite()` and `endWrite()` update the shadow state and call `writeTcaRegister()`, but ignore its boolean result. A failed I2C transaction can leave EXIO3 in the wrong state and corrupt the panel command sequence without an explicit diagnostic.

**Action:** Add a sticky error counter/status or serial diagnostic for failed EXIO3 writes before long-running use. Do not print from every normal pixel transaction; these hooks are for initialization/command transactions, not RGB pixel streaming.

### R4 — Medium: touch coordinate orientation is intentionally uncalibrated

**Evidence:** `Board21BTouch.h` defaults both mirror flags to zero. The driver receives 479x479 as maximum coordinates, but the board's physical rotation/orientation has not been tested. The touch read path clamps values; clamping can conceal a range mismatch.

**Action:** Use the color bars/circle as a coordinate reference and test all four corners plus center. Adjust swap/mirror configuration only after observing raw reported coordinates. Do not infer correct touch alignment from successful I2C initialization alone.

### R5 — Medium: ST7701S init/timing are source-derived, not target-verified

**Evidence:** The 9-bit command/data bus, ST7701S sequence, 480x480 RGB timing and 16 MHz PCLK are internally consistent with the local Arduino_GFX API. The exact panel revision, timing margins, visible orientation and color channel order have not been validated on the physical 2.1B unit.

**Action:** First test stable full-screen color bars and circle geometry. If the image is blank/unstable, check TCA9554 address/CS/reset levels, RGB sync/pixel-clock polarity and porches, then compare the init table against the exact Waveshare board revision. Change one variable at a time.

### R6 — Low: shared GPIO1/GPIO2 must remain an explicit lifecycle constraint

**Evidence:** GPIO1/GPIO2 are used for ST7701S serial initialization and also shared with the microSD interface. The isolated sketch does not initialize SD, which avoids an immediate conflict. The current board adapter has no SD lifecycle management.

**Action:** When SD support is added, document and enforce initialization order and pin reconfiguration. Do not initialize the SD host concurrently with panel command writes.

## API/pin checks completed

- The `Arduino_ESP32RGBPanel` constructor signature matches the argument order used by `createRgbPanel()`.
- RGB data pins are passed in the library's R, G, B parameter order and correspond to the 2.1B map in `pin_config.h`.
- TCA9554 output bits are written before changing direction; only EXIO1–EXIO4 are configured as outputs, leaving the active-high buzzer EXIO8 as an input.
- `TouchDrvCST816` contains the CST820 chip ID (0xB7), and the adapter uses its I2C address 0x15.
- `Arduino_RGB_Display` uses the RGB panel's framebuffer pointer, so the adapter does not itself allocate a second 480x480 RGB565 framebuffer.

## Not yet reviewed / not yet integrated

- Full MotoNav navigation/map/phone-link application path.
- Runtime heap/PSRAM headroom under map rendering and Bluetooth activity.
- Refresh tearing/frame pacing and worst-case CPU load.
- SD card, RTC, IMU, buzzer, charging/battery and sleep/wake paths.
- End-to-end behavior on the physical target board.
