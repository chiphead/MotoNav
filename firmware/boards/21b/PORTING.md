# ESP32-S3-Touch-LCD-2.1B port status

**Status: port in progress — not buildable or hardware-verified yet.**

Target: Waveshare ESP32-S3-Touch-LCD-2.1B (480x480 RGB LCD, ST7701 panel, CST820 capacitive touch, TCA9554 IO expander).

## Porting rules

- Keep this board port isolated from the existing `175` and `185b` targets.
- Do not reuse the 1.75 AMOLED/QSPI display initialization.
- Do not reuse the 1.85B ST77916 panel initialization: it is a different controller and interface.
- Keep the MotoNav Bluetooth protocol and navigation/application logic unchanged wherever possible.
- Do not publish a flashable image until the target build succeeds; mark device testing separately from compilation.

## Hardware references

Use the official Waveshare ESP32-S3-Touch-LCD-2.1B wiki as the source of truth for the exact board revision and pin map:
https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1B

Expected key hardware blocks (verify against the current wiki/revision before coding):
- 480x480 RGB parallel LCD, ST7701
- CST820 touch controller over I2C
- TCA9554 GPIO expander for panel/touch reset and board control signals
- ESP32-S3 with 16 MB flash and 8 MB PSRAM

## Required implementation work

1. Add a dedicated `firmware/boards/21b/MotoNav` target with board-specific pin definitions.
2. Implement panel initialization and RGB timing using the Waveshare ST7701 reference example and matching Arduino_GFX API.
3. Implement the correct TCA9554 startup sequence before panel reset/backlight; confirm the board revision's EXIO assignments.
4. Adapt CST820 I2C touch initialization and coordinate orientation for the 480x480 display.
5. Audit all uses of logical dimensions and the existing 466x466 layout. Choose an explicit scaling/layout strategy instead of changing constants blindly.
6. Add a dedicated build option and output directory only after the board source is complete.
7. Compile with the repository's pinned Arduino CLI / ESP32 Arduino Core, then test on the actual board: boot, display, touch, BLE pairing, navigation updates, map rendering, audio/other peripherals if used.

## Validation record

- Source review: in progress
- Compile: not run
- Physical board test: not run
- Flashable release: not available

This document intentionally does not claim that a working firmware port exists.
