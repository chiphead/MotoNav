#pragma once
#include <stdint.h>

// Waveshare ESP32-S3-Touch-LCD-2.1B (480x480 RGB, ST7701S).
// This is deliberately separate from the 1.75 AMOLED/QSPI and 1.85B/QSPI
// pin maps. Confirm against the exact board revision before enabling builds.

#define LCD_WIDTH 480
#define LCD_HEIGHT 480
#define PANEL_WIDTH 480
#define PANEL_HEIGHT 480

// ST7701S serial initialization bus. Pixel data is sent over RGB, not SPI.
#define LCD_SDA 1
#define LCD_SCLK 2
#define LCD_CS_EXIO 3
#define LCD_RESET_EXIO 1
#define LCD_BL 6

// RGB timing signals.
#define LCD_PCLK 41
#define LCD_DE 40
#define LCD_VSYNC 39
#define LCD_HSYNC 38

// RGB565 data pins, in red/green/blue channel order.
#define LCD_R1 46
#define LCD_R2 3
#define LCD_R3 8
#define LCD_R4 18
#define LCD_R5 17
#define LCD_G0 14
#define LCD_G1 13
#define LCD_G2 12
#define LCD_G3 11
#define LCD_G4 10
#define LCD_G5 9
#define LCD_B1 5
#define LCD_B2 45
#define LCD_B3 48
#define LCD_B4 47
#define LCD_B5 21

// CST820 touch controller (shares the board I2C bus with TCA9554/RTC/IMU).
#define IIC_SDA 15
#define IIC_SCL 7
#define TP_INT 16
#define TP_RESET_EXIO 2
#define BOARD_TOUCH_FLIP_X 0
#define BOARD_TOUCH_FLIP_Y 0

// TCA9554 I/O expander. These EXIO numbers follow Waveshare's 1-based
// naming convention; convert to bit index with (EXIO - 1) when accessing
// the raw TCA9554 registers. Do not drive the expander with an all-ones
// output value: EXIO8 controls the active-high buzzer on this board.
#define TCA9554_I2C_ADDR 0x20
#define BOARD_EXIO_LCD_RESET 1
#define BOARD_EXIO_TOUCH_RESET 2
#define BOARD_EXIO_LCD_CS 3
#define BOARD_EXIO_SD_CS 4
#define BOARD_EXIO_IMU_INT2 5
#define BOARD_EXIO_IMU_INT1 6
#define BOARD_EXIO_RTC_INT 7
#define BOARD_EXIO_BUZZER 8

// microSD: shared SPI clock/data pins are GPIO2/GPIO1; chip select is EXIO4.
// SD_D1 and SD_D2 are not connected on this board.
#define SDMMC_CLK 2
#define SDMMC_CMD 1
#define SDMMC_DATA 42
#define SDMMC_CS_EXIO 4

// Battery voltage sense.
#define BAT_ADC 4

// The 2.1B board has no 1.75-style ES8311 audio path. Do not copy the
// 1.75 I2S/PA pin definitions into this board configuration.
