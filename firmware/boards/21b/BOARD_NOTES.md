# 2.1B board port — hardware findings and implementation status

Target: Waveshare ESP32-S3-Touch-LCD-2.1B, 480x480 RGB565, ST7701S, CST820 touch.

## Confirmed architectural differences

- The display pixel bus is 16-bit parallel RGB. GPIO1/GPIO2 are used for the ST7701S serial initialization path; they are not the pixel data bus.
- ST7701S chip-select and reset are controlled through the TCA9554 I/O expander at I2C address 0x20, not direct ESP32 GPIOs.
- The CST820 uses I2C SDA=GPIO15, SCL=GPIO7, INT=GPIO16; its reset is routed through the expander.
- TCA9554 EXIO names in Waveshare examples are 1-based. Raw register bit index is EXIO-1. EXIO8 is the active-high buzzer, so initializing the expander with an indiscriminate 0xFF output value can sound the buzzer.
- The 2.1B RGB/ST7701S architecture is incompatible with the 1.75 AMOLED/QSPI driver and the 1.85B ST77916/QSPI driver.

## Pin map

See `pin_config.h` for the board-specific pin constants and source comments. The map is a configuration record, not proof that the display driver has been integrated.

## Port status

- [x] Isolated port branch and documented target architecture.
- [x] Added the 2.1B-specific pin map.
- [x] Add an isolated ST7701S init/RGB construction scaffold in `MotoNav/Board21BDisplay.h`, based on a separate open-source implementation of the same Waveshare board. It includes the vendor init command table and RGB timing/pin arguments.
- [x] Add a safe-startup scaffold for TCA9554 LCD reset, touch reset, LCD CS and SD CS. The helper intentionally leaves unused expander pins as inputs to avoid enabling the active-high buzzer.
- [ ] Review the scaffold against the exact Arduino_GFX revision vendored in this repository; constructor and batch-operation API compatibility is not yet verified.
- [ ] Verify expander CS transitions and panel init on hardware. The LCD command bus uses 9-bit serial transactions; CS is routed through TCA9554 while SCL/SDA are direct GPIOs.
- [ ] Integrate rendering without assuming `Arduino_RGB_Display` behaves like the existing QSPI panel. It owns a 480x480 RGB framebuffer; retaining the 466x466 `Arduino_Canvas` and MotoNav double buffer adds more PSRAM usage and needs measured refresh/tearing tests.
- [x] Add an isolated `Board21BTouch.h` adapter using the repository-vendored SensorLib CST816-family driver, which recognizes the CST820 chip ID. It keeps reset on TCA9554 EXIO2 and uses GPIO16 as touch interrupt.
- [ ] Verify touch init, coordinate orientation and tap/swipe behavior on the actual panel.
- [ ] Adapt MotoNav rendering and buffering for 480x480 RGB panel.
- [ ] Add a separate build target only after the board driver is integrated.
- [ ] Compile with the project's pinned Arduino/ESP32 toolchain.
- [ ] Validate on physical hardware.

The display and touch helpers are intentionally isolated and are not yet included by `MotoNav.ino`; the 21b target remains non-buildable until API compatibility, rendering/buffer sizing and the dedicated build script are addressed. No compile or physical-device test has been performed. Do not flash a 1.75 or 1.85B image onto the 2.1B board.


## Isolated hardware bring-up sketch

Added `firmware/boards/21b/bringup/bringup.ino` to test the RGB display and CST820 touch path separately from the large MotoNav application. It initializes TCA9554, creates the ST7701/RGB display, draws color bands and concentric circles, then logs touch coordinates to Serial at 115200 baud.

Suggested test sequence once compiled and flashed:
1. Confirm serial output reports TCA9554 success (otherwise it stops with an error).
2. Check the display shows distinct red/green/blue/white/yellow/magenta/cyan bands and the center circles.
3. Touch several points and check that serial coordinates vary from near 0 to near 479 in both axes.
4. Record any rotation/mirroring discrepancy before porting MotoNav's touch gestures.

**Validation status:** this is a source-level test sketch only. It has not been compiled or flashed; the ST7701 init sequence, RGB timings, expander CS behavior, color order and touch orientation remain unverified on the physical board.


## Build entry point for the isolated test

A dedicated PowerShell wrapper is available at `firmware/build-21b-bringup.ps1`. From the repository's `firmware` directory on Windows, run:

```powershell
.\build-21b-bringup.ps1
```

If Arduino CLI is not on PATH, pass its full path with `-CliPath`; if the installation uses a non-default config, pass `-ConfigFile`. The script uses the same ESP32-S3 FQBN, 16 MB flash, OPI PSRAM, partition scheme and vendored libraries as the existing build script. It only compiles the bring-up sketch and does not package a MotoNav release.

**Important:** no compile was run by the authoring environment. A successful local run is still required before treating the adapter code as compile-validated.


## Follow-up source review (not a compile result)

Compared the adapter against the repository's vendored Arduino_GFX API and a separate public implementation of the same Waveshare 2.1-inch board:

- The RGB panel constructor argument order matches the vendored `Arduino_ESP32RGBPanel` declaration. The adapter keeps the 480x480, 16-bit RGB bus at a conservative 16 MHz pixel clock with a 480x10-pixel bounce buffer.
- The display's ST7701S command/data bus is 9-bit software SPI on GPIO2/GPIO1, while chip-select is controlled through TCA9554 EXIO3. This matches the board's documented split between RGB pixel transfer and the boot-time serial initialization bus. The SD card shares GPIO1/GPIO2 and uses EXIO4 as its chip-select; the bring-up sketch intentionally does not initialize SD.
- The TCA9554 setup now explicitly clears its polarity-inversion register before configuring only LCD reset, touch reset, LCD CS and SD CS as outputs. EXIO8 (the active-high buzzer) remains an input, avoiding an accidental continuous tone.
- RGB sync polarity is set to active-low explicitly. If the first hardware test shows a blank or unstable image, timing polarity/porches and the ST7701S sequence remain items to compare against the exact board revision; these settings have not been measured on the target device.

The reference used for the board wiring is Waveshare's [ESP32-S3-Touch-LCD-2.1 wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1). A separate implementation with the same 480x480 ST7701S/CST820/TCA9554 hardware is available at [murr2k/esp32-s3-touch-lcd-2.1](https://github.com/murr2k/esp32-s3-touch-lcd-2.1). Neither reference proves that this Arduino_GFX adapter compiles or runs unchanged.
