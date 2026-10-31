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
