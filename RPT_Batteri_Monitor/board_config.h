#pragma once

#include <Arduino.h>
#include "driver/gpio.h"

/**
 * ============================================================================
 * Hardware Pin Definitions for Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
 * Display: 7.0 inch 800x480 RGB LCD
 * ============================================================================
 */

// -----------------------------------------------------------------------------
// CAN Bus Interface (TWAI)
// Note: On the 7" board, GPIO 19 & 20 are routed to the onboard TJA1051 CAN
// transceiver via an FSUSB42 multiplexer controlled by CH422G EXIO5.
// CH422G EXIO5 MUST BE PULLED HIGH to enable CAN mode!
//
// Point-to-Point Mode:
// Direct dedicated connection to RPT battery CAN port (no inverter on CAN bus).
// In this mode, ESP32 must provide hardware ACK bits so the battery's CAN
// controller does not halt with ACK errors. Firmware transmits zero data frames.
// -----------------------------------------------------------------------------
#define BOARD_CAN_TX_PIN            GPIO_NUM_20
#define BOARD_CAN_RX_PIN            GPIO_NUM_19
#define BOARD_CAN_DEFAULT_BAUDRATE  500000      // 500 kbit/s (standard Deye/Pylontech)
#define BOARD_CAN_POINT_TO_POINT    true        // Direct point-to-point to battery CAN

// -----------------------------------------------------------------------------
// I2C Interface (Shared: CH422G I/O Expander, GT911 Touch, PCF85063 RTC)
// -----------------------------------------------------------------------------
#define BOARD_I2C_SDA_PIN           GPIO_NUM_8
#define BOARD_I2C_SCL_PIN           GPIO_NUM_9
#define BOARD_I2C_FREQ_HZ           400000      // 400 kHz Fast-Mode

// CH422G I2C 7-bit Addresses:
// 0x48 >> 1 = 0x24 (WR-SET: system parameter register)
// 0x46 >> 1 = 0x23 (WR-OC: open drain output register)
// 0x70 >> 1 = 0x38 (WR-IO: general purpose output register EXIO0-7)
// 0x4D >> 1 = 0x26 (RD-IO: general purpose input register)
#define CH422G_I2C_ADDR_WR_SET      0x24
#define CH422G_I2C_ADDR_WR_OC       0x23
#define CH422G_I2C_ADDR_WR_IO       0x38
#define CH422G_I2C_ADDR_RD_IO       0x26

// CH422G EXIO Pin Mask Assignments on ESP32-S3-Touch-LCD-7
#define CH422G_EXIO1_TP_RST         (1 << 1)    // GT911 Touch reset (active low)
#define CH422G_EXIO2_LCD_BL         (1 << 2)    // LCD Backlight enable (active high)
#define CH422G_EXIO3_LCD_RST        (1 << 3)    // LCD Reset (active low)
#define CH422G_EXIO4_SD_CS          (1 << 4)    // MicroSD Chip Select (active low)
#define CH422G_EXIO5_CAN_SEL        (1 << 5)    // High = CAN Mode, Low = USB Mode

// -----------------------------------------------------------------------------
// MicroSD Card (TF Card) Interface (SPI)
// -----------------------------------------------------------------------------
#define BOARD_SD_MOSI_PIN           GPIO_NUM_11
#define BOARD_SD_SCK_PIN            GPIO_NUM_12
#define BOARD_SD_MISO_PIN           GPIO_NUM_13
#define BOARD_SD_CS_PIN             -1          // Managed via CH422G EXIO4

// -----------------------------------------------------------------------------
// Touch Screen (GT911)
// -----------------------------------------------------------------------------
#define BOARD_TOUCH_IRQ_PIN         GPIO_NUM_4
#define BOARD_TOUCH_I2C_ADDR        0x5D        // Default address (or 0x14)

// -----------------------------------------------------------------------------
// RS485 Interface
// -----------------------------------------------------------------------------
#define BOARD_RS485_TX_PIN          GPIO_NUM_15
#define BOARD_RS485_RX_PIN          GPIO_NUM_16

// -----------------------------------------------------------------------------
// 7.0-inch 800x480 RGB Display (ST7262 / EK9716 timing)
// -----------------------------------------------------------------------------
#define LCD_WIDTH                   800
#define LCD_HEIGHT                  480
#define LCD_PIXEL_CLOCK_HZ          (16 * 1000 * 1000) // 16 MHz

#define LCD_PIN_VSYNC               GPIO_NUM_3
#define LCD_PIN_HSYNC               GPIO_NUM_46
#define LCD_PIN_DE                  GPIO_NUM_5
#define LCD_PIN_PCLK                GPIO_NUM_7

// RGB565 16-bit Data Bus
#define LCD_PIN_DATA0               GPIO_NUM_14 // B3
#define LCD_PIN_DATA1               GPIO_NUM_38 // B4
#define LCD_PIN_DATA2               GPIO_NUM_18 // B5
#define LCD_PIN_DATA3               GPIO_NUM_17 // B6
#define LCD_PIN_DATA4               GPIO_NUM_10 // B7
#define LCD_PIN_DATA5               GPIO_NUM_39 // G2
#define LCD_PIN_DATA6               GPIO_NUM_0  // G3
#define LCD_PIN_DATA7               GPIO_NUM_45 // G4
#define LCD_PIN_DATA8               GPIO_NUM_48 // G5
#define LCD_PIN_DATA9               GPIO_NUM_47 // G6
#define LCD_PIN_DATA10              GPIO_NUM_21 // G7
#define LCD_PIN_DATA11              GPIO_NUM_1  // R3
#define LCD_PIN_DATA12              GPIO_NUM_2  // R4
#define LCD_PIN_DATA13              GPIO_NUM_42 // R5
#define LCD_PIN_DATA14              GPIO_NUM_41 // R6
#define LCD_PIN_DATA15              GPIO_NUM_40 // R7

// Timing parameters for 800x480 RGB Panel
#define LCD_TIMING_HPW              4
#define LCD_TIMING_HBP              8
#define LCD_TIMING_HFP              8
#define LCD_TIMING_VPW              4
#define LCD_TIMING_VBP              16
#define LCD_TIMING_VFP              16
