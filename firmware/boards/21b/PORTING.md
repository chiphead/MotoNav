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


## 2026-11-01 source inspection

The existing 1.85B implementation is not a valid display-driver starting point for this board:

- 1.85B constructs `Arduino_ESP32QSPI` and derives its panel from `Arduino_ST77916`.
- 2.1B is a 480x480 ST7701 RGB-parallel panel. Its pixel bus is the RGB set below, with PCLK/DE/VSYNC/HSYNC timing signals. Do not carry over the QSPI bus, ST77916 init table, panel-ID probing, or QSPI clock preference.
- The existing 1.85B touch path instantiates `TouchDrvCST816`. The 2.1B uses CST820; the driver class/API and reset/interrupt behavior must be checked against the exact Waveshare demo/library version before reuse.
- The current 1.85B `setup()` initializes Wire and touch before `gfx->begin()`. On 2.1B, the TCA9554 expander must be brought up first if it controls LCD reset and touch reset. The exact EXIO mapping and expander polarity must be confirmed from the official 2.1B demo, not inferred from the 1.85B board.

### 2.1B RGB and control pin inventory

| Signal | GPIO / connection |
|---|---:|
| LCD backlight | GPIO6 |
| LCD reset | TCA9554 EXIO1 |
| LCD SDA / SCL / CS | GPIO1 / GPIO2 / TCA9554 EXIO3 |
| RGB PCLK / DE / VSYNC / HSYNC | GPIO41 / GPIO40 / GPIO39 / GPIO38 |
| RGB B1..B5 | GPIO5, 45, 48, 47, 21 |
| RGB G0..G5 | GPIO14, 13, 12, 11, 10, 9 |
| RGB R1..R5 | GPIO46, 3, 8, 18, 17 |
| Touch SDA / SCL | GPIO15 / GPIO7 |
| Touch interrupt | GPIO16 |
| Touch reset | TCA9554 EXIO2 |

These are a pin inventory, not yet a compilable Arduino_GFX constructor. Confirm the RGB bit ordering and ST7701 init sequence against Waveshare's current 2.1B demo before implementing the panel object.

### Porting boundary

The BLE protocol, map rendering, media/HID behavior, and most application logic should remain shared from the 1.85B lineage where practical. Board-specific code must be isolated behind a 2.1B display/touch/power initialization layer. Do not copy the 1.85B QSPI panel class into the 2.1B target and do not enable a build.ps1 target until the RGB panel and CST820 path compile together.

### Current verification

- Repository source inspection: complete for the 1.85B panel constructor, touch initialization, and setup ordering.
- 2.1B pin inventory: documented from the board specification; exact expander sequence and RGB panel timing/init still need verification against the official demo.
- Firmware implementation: not yet started.
- Arduino compile: not run.
- Hardware validation: not run.


## Reference implementation audit (2026-11-01)

A public Arduino/PlatformIO template for the exact 2.1-inch Waveshare board was inspected as a secondary implementation reference:
`saranyim/Template_waveshare_ESP32_S3_Touch_LCD_2.1` (not a first-party Waveshare source and not yet validated on this MotoNav fork).

Useful implementation findings from that reference:

- ST7701 command transport is a separate 3-wire/9-bit SPI path on GPIO2 (clock) and GPIO1 (MOSI); the panel RGB pixel stream uses the 16 data GPIOs plus PCLK/DE/HSYNC/VSYNC. It is not a normal SPI display.
- TCA9554 is at I2C address 0x20. Its EXIO1 controls panel reset, EXIO2 controls CST820 reset, and EXIO3 controls ST7701 command-interface CS. The reference uses EXIO numbers as 1-based pin identifiers.
- CST820 uses I2C address 0x15, SDA GPIO15, SCL GPIO7, and interrupt GPIO16. Its reset is performed through TCA9554 EXIO2.
- Reference RGB timing values are 16 MHz PCLK, H pulse/back/front porch 8/10/50, V pulse/back/front porch 3/8/8, with two frame buffers and a bounce buffer option.
- The reference ST7701 init sequence writes pixel-format register 0x3A with value 0x66 (RGB666), while its config header declares 16-bit RGB pixel/data widths. This is a material configuration mismatch to resolve deliberately; do not blindly copy either value. A 16-bit RGB565 framebuffer can still require a panel-side RGB666 serial configuration depending on the RGB bus wiring and driver conversion path.
- TCA9554 EXIO8 behavior is not documented consistently across the inspected examples. Keep all expander outputs in a known safe state and verify buzzer/panel-enable behavior against the exact board revision before enabling any EXIO8 output.

Reference source files inspected: `Display_ST7701.h/.cpp`, `TCA9554PWR.h/.cpp`, and `Touch_CST820.h/.cpp` in that repository. Treat their initialization/timing as a reference baseline, not as code already integrated into MotoNav.

### Updated implementation sequence

1. Extract a minimal board-only bring-up from the reference: I2C scan, TCA9554 safe initialization, ST7701 command bus/init, RGB panel creation, solid-color/full-screen test.
2. Separately validate CST820 reset, I2C reads, coordinates, and IRQ/polling reliability.
3. Only then adapt MotoNav's framebuffer flush and 480x480 canvas scaling.
4. Keep the BLE/map/application logic untouched until the display/touch smoke test is stable.


## 2026-11-01 board pin contract added

Added `firmware/boards/21b/MotoNav/pin_config.h` as a dedicated 2.1B pin inventory.

- Includes the 480x480 geometry, RGB timing/data pins, ST7701 command pins, CST820 I2C/IRQ pins, and TCA9554 EXIO assignments.
- Adds the documented microSD signals: GPIO42 data, GPIO2 clock, GPIO1 command, EXIO4 chip select.
- Explicitly avoids carrying over the 1.75 ES8311/I2S pin definitions.
- EXIO identifiers are documented as 1-based; raw TCA9554 register access must convert them to bit indices.
- This is a hardware pin contract only. It is not yet wired to a display driver and must not be treated as a buildable board target.

The official board documentation lists EXIO1=LCD reset, EXIO2=touch reset, EXIO3=LCD command CS, EXIO4=SD CS, EXIO5/6=IMU interrupts, EXIO7=RTC interrupt, and EXIO8=buzzer. Because EXIO8 is the buzzer control, any expander initialization must explicitly hold it inactive before configuring outputs; never write an unreviewed all-ones output mask.

### Updated verification

- 2.1B pin inventory: added to the board-specific source tree.
- TCA9554 driver/startup: not implemented.
- ST7701 RGB panel driver/init: not implemented.
- CST820 runtime integration: not implemented.
- Build target: intentionally not enabled.
- Compile and physical board test: not run.
