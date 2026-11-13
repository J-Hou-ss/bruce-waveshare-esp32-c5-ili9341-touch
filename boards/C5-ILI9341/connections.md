# Waveshare ESP32-C5-WIFI6-KIT + 2.8" ILI9341 touch screen: wiring

Environment: `c5-ili9341`. Screen: 2.8" 240x320 SPI module with XPT2046 touch (LCDwiki MSP2807, "2.8 TFT SPI 240x320 V1.2").
Use the header labels printed on the board (IO1, IO2, ...).

## Screen and touch (required)

| Screen pin | Board pin | Note |
| --- | :---: | --- |
| VCC | 3V3 | |
| GND | GND | |
| CS | IO5 | |
| RESET | IO4 | driven by the firmware |
| DC | IO3 | |
| SDI (MOSI) | IO2 | shared with T_DIN |
| SCK | IO1 | shared with T_CLK |
| LED | IO6 | backlight |
| SDO (MISO) | IO7 | shared with T_DO |
| T_CLK | IO1 | |
| T_CS | IO23 | |
| T_DIN | IO2 | |
| T_DO | IO7 | |
| T_IRQ | not connected | |

## Optional modules

All SPI modules share the screen's bus: SCK IO1, MOSI IO2, MISO IO7.

| Module | CS | Other |
| --- | :---: | --- |
| SD card | IO10 | |
| CC1101 | IO9 | GDO0 IO8 |
| NRF24 | IO9 | CE IO8 |
| W5500 | IO9 | INT IO8 |

CC1101, NRF24 and W5500 use the same two pins. Fit one at a time, or add a switch on CS and GDO0/CE.

| Function | Pins |
| --- | --- |
| I2C (PN532, FM radio, ...) | SDA IO24, SCL IO25 |
| GPS / CH9329 serial | RX IO24, TX IO25 (shared with I2C) |
| IR receiver | IO26 |
| IR LED | IO0 |
| RGB LED | IO27 (on the board) |

These are defaults. Most can be changed on the device in Config > Pins.

## Pins to leave alone

| Pin | Why |
| --- | --- |
| IO6 as a clock or data line | The board ties IO6 to its battery-sense divider, which has a 100 nF capacitor to ground. Fast signals do not get through. This is why the stock Bruce `esp32-c5-tft` build (SPI clock on IO6) shows a white screen on this board. A steady level is fine, so IO6 is the backlight here. |
| IO15 (label NC/IO15) | PSRAM chip select inside the module. Configuring it locks the chip up. |
| IO28 | BOOT button, strapping pin. |
| IO11, IO12 | UART0, the USB-C port labelled UART. |
| IO13, IO14 | Native USB, the USB-C port labelled USB. Bruce's serial log comes out here. |

## Touch

- First boot shows four corner arrows. Tap each one, right on the arrow.
- To calibrate again: keep a finger on the middle of the screen while the board starts, until it says to lift it.
- A calibration that comes out too narrow (for example after a tap missed an arrow) is not saved; the arrows start again.
- Left 40% of the screen = previous, middle 20% = select, right 40% = next. The top-left corner (the X) = back.

## Flashing over another Bruce build

Erase the flash first (`esptool --chip esp32c5 erase-flash`). Bruce saves its pin settings on first boot,
and a saved copy from another build overrides the pins compiled into this one.
