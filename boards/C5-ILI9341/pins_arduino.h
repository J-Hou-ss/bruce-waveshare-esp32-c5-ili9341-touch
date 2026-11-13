#ifndef C5_ILI9341_PINS_ARDUINO_H
#define C5_ILI9341_PINS_ARDUINO_H

// Bruce pin map for:
//   Waveshare ESP32-C5-WIFI6-KIT (ESP32-C5-WROOM-1 / -1U, N16R8 or N32R8)
//   + 2.8" ILI9341 240x320 SPI screen with XPT2046 resistive touch (LCDwiki MSP2807, "2.8 TFT SPI 240x320 V1.2")
//
// Why this is not the stock Bruce esp32-c5-tft map:
//   On the Waveshare kit GPIO6 is tied (R39, 0 ohm) to the battery-sense divider, which has a 100 nF
//   filter capacitor to GND. The stock map uses GPIO6 as the SPI clock, so the clock never reaches
//   the screen and it stays white. GPIO6 is fine for a steady level, so it is the backlight here.
//   Display pins below are the ones Waveshare's own ILI9341 demo uses (SCK 1, MOSI 2, CS 5, DC 3, RST 4, BL 6).
//
// Pins never to use on this module:
//   GPIO15  - PSRAM chip select inside the module (header label "NC/IO15"). Configuring it locks the chip up.
//   GPIO28  - BOOT button / strapping pin.
//   GPIO27  - on-board WS2812 RGB LED (used as Bruce's RGB LED).
//   GPIO11/12 - UART0 (the port labelled UART), GPIO13/14 - native USB (the port labelled USB).
//
// Wiring (screen pin -> header label):
//   VCC -> 3V3      GND -> GND
//   CS -> IO5       RESET -> IO4      DC -> IO3      SDI(MOSI) -> IO2      SCK -> IO1      LED -> IO6
//   SDO(MISO) -> IO7
//   T_CLK -> IO1 (with SCK)   T_CS -> IO23   T_DIN -> IO2 (with SDI)   T_DO -> IO7 (with SDO)   T_IRQ -> not connected
// Optional modules, all on the same SPI bus (SCK 1, MOSI 2, MISO 7):
//   SD card CS -> IO10      CC1101 / NRF24 / W5500 CS -> IO9, GDO0 / CE / INT -> IO8
// Optional, off the SPI bus:
//   I2C (PN532, FM radio...) SDA -> IO24, SCL -> IO25. GPS and CH9329 serial share these two pins.
//   IR receiver -> IO26      IR LED -> IO0

#include "soc/soc_caps.h"
#include <stdint.h>

#define PIN_RGB_LED 27
static const uint8_t LED_BUILTIN = SOC_GPIO_PIN_COUNT + PIN_RGB_LED;
#define BUILTIN_LED LED_BUILTIN
#define LED_BUILTIN LED_BUILTIN
#define RGB_BUILTIN LED_BUILTIN
#define RGB_BRIGHTNESS 64

static const uint8_t TX = 11;
static const uint8_t RX = 12;
static const uint8_t USB_DM = 13;
static const uint8_t USB_DP = 14;
static const uint8_t SDA = 24;
static const uint8_t SCL = 25;
static const uint8_t SS = 9;
static const uint8_t MOSI = 2;
static const uint8_t MISO = 7;
static const uint8_t SCK = 1;
static const uint8_t A0 = 1;
static const uint8_t A1 = 2;
static const uint8_t A2 = 3;
static const uint8_t A3 = 4;
static const uint8_t A4 = 5;
static const uint8_t A5 = 6;

// The low-power I2C peripheral is fixed to GPIO4/5 in the chip. Those are display pins here, so it is not used.
static const uint8_t LP_SDA = 4;
static const uint8_t LP_SCL = 5;
#define WIRE1_PIN_DEFINED
#define SDA1 LP_SDA
#define SCL1 LP_SCL
#define LP_RX 12
#define LP_TX 11

#define HAS_RGB_LED 1
#define LED_ORDER GRB
#define LED_TYPE_IS_RGBW 1
#define LED_COUNT 1
#define LED_TYPE WS2812
#define LED_COLOR_STEP 15
#define RGB_LED 27

/* Buses */
#define SERIAL_TX 11
#define SERIAL_RX 12
#define GROVE_SDA 24 // not 4/5 as in the stock map: those are TFT_RST and TFT_CS on this board
#define GROVE_SCL 25
#define SPI_SCK_PIN 1
#define SPI_MOSI_PIN 2
#define SPI_MISO_PIN 7
#define SPI_SS_PIN 9

/* Screen */
#define HAS_SCREEN 1
#define ROTATION 1
#define MINBRIGHT (uint8_t)1
#define USER_SETUP_LOADED 1
#define ILI9341_DRIVER 1
#define TFT_HEIGHT 320
#define TFT_WIDTH 240
#define TFT_BACKLIGHT_ON 1
#define TFT_BL 6  // steady level / slow PWM only: GPIO6 carries the battery-sense capacitor
#define TFT_RST 4 // a real reset pulse from a GPIO (the stock map ties RESET to the board's EN pin)
#define TFT_DC 3
#define TFT_MISO SPI_MISO_PIN
#define TFT_MOSI SPI_MOSI_PIN
#define TFT_SCLK SPI_SCK_PIN
#define TFT_CS 5
#define SMOOTH_FONT 1
#define SPI_FREQUENCY 40000000 // same speed Waveshare's demo runs on these pins; drop to 20000000 if the picture glitches
#define SPI_READ_FREQUENCY 16000000
#define SPI_TOUCH_FREQUENCY 2500000

/* Touch (XPT2046 on the screen's SPI bus, calibrated on first boot) */
#define USE_TFT_eSPI_TOUCH 1
#define HAS_TOUCH 1
#define TOUCH_CS 23
#define TOUCH_INT -1
#define BTN_ACT LOW
#define DEEPSLEEP_WAKEUP_PIN -1 // no button and T_IRQ is not wired, so there is nothing to wake the board

/* Optional modules. Defaults only: Bruce saves pins on first boot and most can be changed in Config > Pins. */
// CH9329 (BadUSB) and GPS serial share the I2C pins, as in the stock map
#define BAD_RX 24
#define BAD_TX 25
#define GPS_SERIAL_TX 25
#define GPS_SERIAL_RX 24
// Infrared (the stock map has the IR LED on GPIO3, which is TFT_DC here)
#define RXLED 26
#define TXLED 0
#define LED_ON HIGH
#define LED_OFF LOW
// SD card
#define SDCARD_CS 10
#define SDCARD_SCK SPI_SCK_PIN
#define SDCARD_MISO SPI_MISO_PIN
#define SDCARD_MOSI SPI_MOSI_PIN
// CC1101, NRF24 and W5500 share one chip-select and one signal pin: fit one at a time, or add a switch
#define CC1101_GDO0_PIN 8
#define CC1101_SS_PIN 9
#define CC1101_MOSI_PIN SPI_MOSI_PIN
#define CC1101_SCK_PIN SPI_SCK_PIN
#define CC1101_MISO_PIN SPI_MISO_PIN
#define NRF24_CE_PIN 8
#define NRF24_SS_PIN 9
#define NRF24_MOSI_PIN SPI_MOSI_PIN
#define NRF24_SCK_PIN SPI_SCK_PIN
#define NRF24_MISO_PIN SPI_MISO_PIN
#define W5500_INT_PIN 8
#define W5500_SS_PIN 9
#define W5500_MOSI_PIN SPI_MOSI_PIN
#define W5500_SCK_PIN SPI_SCK_PIN
#define W5500_MISO_PIN SPI_MISO_PIN

#endif
