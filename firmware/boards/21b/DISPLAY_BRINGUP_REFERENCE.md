# 2.1B Display Bring-up Parameters (Reference Capture)

This note records concrete values found in a public Arduino example for the Waveshare 2.1-inch board. Treat it as a cross-check/reference, not as a substitute for the exact current Waveshare demo archive. Before merging a driver, compare the register sequence and RGB timings with the archive shipped for the board revision in hand.

Reference: https://gist.github.com/haswalt/bf35e17fed10646d620f886d35385a95

## RGB timing reference

- Resolution: 480 x 480
- Data width: 16 bits, RGB565
- Pixel clock: 16 MHz
- HSYNC: pulse 8, back porch 50, front porch 10
- VSYNC: pulse 8, back porch 8, front porch 3
- RGB signal pins: HSYNC 38, VSYNC 39, DE 40, PCLK 41
- Data order passed to the RGB panel driver:
  - B0..B4: GPIO 5, 45, 48, 47, 21
  - G0..G5: GPIO 14, 13, 12, 11, 10, 9
  - R0..R4: GPIO 46, 3, 8, 18, 17
- Example uses PSRAM framebuffers and two framebuffers. It also configures a 10-line bounce buffer and 64-byte PSRAM transfer alignment.

## ST7701S command link

- Command SDA/MOSI: GPIO1
- Command SCL: GPIO2
- CS: TCA9554 EXIO3
- Reset: TCA9554 EXIO1
- Example uses SPI2_HOST, command_bits=1, address_bits=8, mode 0, 40 MHz, no hardware CS.
- The example toggles the expander CS around the complete init sequence.
- Panel sleep-out (0x11) is followed by a 480 ms delay; display-on (0x29) follows after an additional delay.
- RGB panel is created and initialized after the ST7701 command sequence.

## TCA9554 startup

The example uses I2C address 0x20 and GPIO I2C pins SDA=15, SCL=7. It initializes the expander before the display, enables board power via EXIO8, then performs panel reset/init. The actual polarity and safe state must be checked against the schematic and current official driver. Do not blindly write 0xFF to the output register: the expander also drives board functions.

## Touch reference

The example uses CST820 on the same I2C bus (SDA=15, SCL=7) and a dedicated interrupt line. The reset is controlled through TCA9554 EXIO2. Raw coordinates must be measured on the physical panel before setting any X/Y flip or rotation correction.

## Important integration warning

The sample's custom Arduino_GFX wrapper implements some drawing primitives by allocating temporary heap buffers and calling draw_bitmap for individual operations. That approach is useful as a proof of life, but it is not an appropriate high-performance adapter for MotoNav's full-screen map renderer. The production adapter should use the RGB framebuffer and a bounded present/flush strategy, with explicit PSRAM/DMA allocation checks and no per-pixel heap allocations.

## Next implementation gate

Do not add the 21b option to build.ps1 yet. First build a minimal standalone sketch using the exact Waveshare libraries/sequence and verify:
1. TCA9554 responds at 0x20.
2. Backlight PWM responds on GPIO6.
3. RGB colour bars fill all 480x480 pixels without drift/tearing.
4. CST820 returns stable coordinates across all four corners.
5. Repeated reset and power cycles do not leave the panel blank.
