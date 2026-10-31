# ESP32-S3-Touch-LCD-2.1B Port Architecture

Status: hardware architecture researched; firmware driver implementation and build validation are still pending.

Target: Waveshare ESP32-S3-Touch-LCD-2.1B (480x480, RGB parallel, ST7701S, CST820).

## 1. Why the existing board ports cannot be reused as display drivers

- Board 175 uses a QSPI AMOLED bus and its AXP2101 power assumptions.
- Board 185b uses a QSPI bus and ST77916 panel initialization. Its 360x360 physical panel is drawn through a 466x466 logical canvas.
- Board 21b uses an RGB timing bus and an ST7701S command interface. The RGB data path and the panel command/init path are separate. Reusing either old panel class is incorrect.

The MotoNav application logic (BLE protocol, navigation state, map rendering and most UI drawing) should be retained where practical. Board bring-up must be isolated behind a board-specific display/touch/power layer.

## 2. Target display and touch wiring

The following mapping is taken from Waveshare's 2.1 board documentation and board references. It must be checked against the exact demo revision and schematic before coding the final driver.

### RGB panel

| Signal | ESP32-S3 / expander |
|---|---:|
| LCD backlight | GPIO6 |
| LCD reset | TCA9554 EXIO1 |
| LCD command SDA | GPIO1 |
| LCD command SCL | GPIO2 |
| LCD command CS | TCA9554 EXIO3 |
| PCLK | GPIO41 |
| DE | GPIO40 |
| VSYNC | GPIO39 |
| HSYNC | GPIO38 |
| B1..B5 | GPIO5, 45, 48, 47, 21 |
| G0..G5 | GPIO14, 13, 12, 11, 10, 9 |
| R1..R5 | GPIO46, 3, 8, 18, 17 |

Panel geometry: 480x480, RGB565 output over a 16-bit RGB bus. ST7701S initialization is sent over the separate 3-wire command interface; only after that should the RGB panel timing driver start.

### Touch

| Signal | ESP32-S3 / expander |
|---|---:|
| I2C SDA | GPIO15 |
| I2C SCL | GPIO7 |
| Interrupt | GPIO16 |
| Touch reset | TCA9554 EXIO2 |
| Controller | CST820 |

The display command bus and touch I2C bus are not the same pin pair. Do not inherit the 185b touch pin macros.

### Other onboard devices

The board also exposes a TCA9554 IO expander, QMI8658 IMU, PCF85063 RTC, microSD and battery circuitry. These are not prerequisites for the first screen-on milestone. Keep them out of the initial display bring-up path unless the board's official init requires them.

## 3. Recommended implementation sequence

1. Add a dedicated `firmware/boards/21b/MotoNav` source directory; do not modify 175 or 185b.
2. Implement TCA9554 initialization first. Set a safe idle output state before enabling output directions; the expander also controls active-low reset lines and other board functions.
3. Implement ST7701S command initialization using the exact Waveshare 2.1 demo sequence and its required 3-wire SPI timing.
4. Start the ESP32-S3 RGB panel with the documented 480x480 timing and 16 data pins. Confirm RGB565 colour bars before connecting the MotoNav renderer.
5. Implement CST820 touch using GPIO15/7/16 and EXIO2 reset. Validate raw coordinates and orientation separately.
6. Adapt MotoNav's canvas/flush path to the RGB driver's framebuffer/present API. The current QSPI-specific bus-sharing and panel push code must not be carried over blindly.
7. Only after screen and touch tests pass, add a 21b build target and reconnect the application features.

## 4. Memory and performance considerations

A 480x480 RGB565 frame is 460,800 bytes (450 KiB). Double buffering requires about 900 KiB before any additional map/canvas buffers. The target has 8 MiB PSRAM, but the actual framebuffer allocation, alignment and DMA-capable memory constraints must be tested on hardware. Avoid assuming that the 185b dual-core QSPI flush implementation is suitable for RGB DMA.

## 5. Build system policy

Do not add `21b` to the `ValidateSet` in `firmware/build.ps1` until the dedicated source tree and correct RGB/ST7701 implementation exist. A selectable board target that still compiles the 185b driver would be misleading.

## 6. Reference material

- Waveshare board wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1
- Waveshare demo archive is linked from the wiki Resources section.
- A third-party ESP-IDF implementation with documented ST7701S/RGB, TCA9554 and CST820 bring-up: https://github.com/murr2k/esp32-s3-touch-lcd-2.1
- The third-party project is useful for architecture and pin cross-checking, not a drop-in MotoNav dependency.

## 7. Validation record

- [x] Confirmed 175 is QSPI AMOLED and not compatible.
- [x] Confirmed 185b is QSPI/ST77916 and not compatible.
- [x] Identified the 21B RGB/ST7701S + TCA9554 + CST820 architecture.
- [ ] Verify every signal and expander bit against the exact Waveshare demo/schematic revision.
- [ ] Implement and compile the board-specific display driver.
- [ ] Verify colour bars and touch coordinates on physical hardware.
- [ ] Integrate MotoNav UI and BLE/navigation application.
