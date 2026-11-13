# Bruce firmware for Waveshare ESP32-C5-WIFI6-KIT + 2.8" ILI9341 touch screen

A ready-to-flash build of the [Bruce firmware](https://github.com/BruceDevices/firmware) for the Waveshare ESP32-C5-WIFI6-KIT with a 2.8" ILI9341 SPI touch screen. Touch only, no buttons.

Bruce's stock `esp32-c5-tft` build shows a white screen on this board. It clocks the screen on GPIO6, which Waveshare ties to the battery-sense circuit (100 nF to ground), so the clock never reaches the screen. This build uses the pins from Waveshare's own ILI9341 demo instead.

**Download:** [Bruce-C5-ILI9341.bin](https://github.com/J-Hou-ss/bruce-waveshare-esp32-c5-ili9341-touch/releases/latest/download/Bruce-C5-ILI9341.bin) · [SHA256SUMS.txt](https://github.com/J-Hou-ss/bruce-waveshare-esp32-c5-ili9341-touch/releases/latest/download/SHA256SUMS.txt)

| | |
| --- | --- |
| Board | Waveshare ESP32-C5-WIFI6-KIT (ESP32-C5-WROOM-1 or -1U module) |
| Screen | 2.8" ILI9341 SPI module, 240x320, with XPT2046 resistive touch (MSP2807) |
| Input | Touch: left 40% = previous, middle 20% = select, right 40% = next, top-left corner = back |
| Tested | Display, menus and touch on real hardware. The optional modules (SD card, CC1101, NRF24, PN532) have default pins assigned but are not yet tested with this build. |
| Base | Bruce `main` at commit [`a59213f3`](https://github.com/BruceDevices/firmware/commit/a59213f3ec6cd302fa32c2042b2700c9c24d991a) (2026-09-09). Unofficial build. |

## Wiring

Use the header labels printed on the board. The touch controller shares the screen's SPI bus.

| Screen pin | Board pin |
| --- | :---: |
| VCC | 3V3 |
| GND | GND |
| CS | IO5 |
| RESET | IO4 |
| DC | IO3 |
| SDI (MOSI) and T_DIN | IO2 |
| SCK and T_CLK | IO1 |
| LED | IO6 |
| SDO (MISO) and T_DO | IO7 |
| T_CS | IO23 |
| T_IRQ | not connected |

**Never connect anything to IO15** (header label `NC/IO15`). It is the PSRAM chip select inside the module, and configuring it locks the chip up.

Optional modules and the other pins to leave alone are in the [full wiring sheet](boards/C5-ILI9341/connections.md).

## Flash

1. Download the image above.
2. Connect the USB-C socket labelled **UART**.
3. Erase the flash, then write the image at address `0x0` with [esptool](https://github.com/espressif/esptool):

   ```
   esptool --chip esp32c5 --port COM5 erase-flash
   esptool --chip esp32c5 --port COM5 write-flash 0x0 Bruce-C5-ILI9341.bin
   ```

   Replace `COM5` with your port. These are esptool 5 command names; on esptool 4 use underscores (`erase_flash`, `write_flash`).

4. On first boot, tap each of the four corner arrows to calibrate the touch screen.

Good to know:

- **Always erase first.** Bruce saves its pin settings on first boot, and settings saved by another build override the ones in this image.
- To keep what is on the board now, back it up before erasing: `esptool --chip esp32c5 --port COM5 read-flash 0 ALL backup.bin`.

## Build from source

You need [PlatformIO](https://platformio.org/) (the CLI or the VS Code extension).

```
git clone https://github.com/BruceDevices/firmware.git bruce
cd bruce
git checkout a59213f3ec6cd302fa32c2042b2700c9c24d991a
```

Copy `boards/C5-ILI9341` from this repository into Bruce's `boards/` folder. Then, from inside the Bruce folder, apply the patch and build:

```
git apply path/to/this/repo/patches/0001-pinouts-c5-ili9341-selector.patch
pio run -e c5-ili9341
```

The build writes `Bruce-c5-ili9341.bin` to the Bruce folder, ready to flash at `0x0`.

The patch adds 2 lines to `boards/pinouts/pins_arduino.h` and changes nothing for Bruce's other boards. Always name the environment: a plain `pio run` builds every board Bruce supports.

## Troubleshooting

- **Touch responds in one corner only.** The saved calibration is bad. Hold a finger on the middle of the screen while the board starts, until it says to lift it, then tap the four arrows again.
- **White screen.** Check the wiring against the table above, in particular that the screen's SCK goes to IO1 and not IO6.

<details>
<summary>Build errors on Windows</summary>

- **`MSys/Mingw is not supported`.** Run PlatformIO from PowerShell or the PlatformIO terminal in VS Code, not from Git Bash.
- **`cannot find -lnet80211`.** Bruce's `patch.py` calls `pio` to weaken one symbol in the Wi-Fi library. If `pio` is not on your PATH, that step fails after renaming the library to `libnet80211.a.old`. Add PlatformIO's `Scripts` folder (`bin` on Linux and macOS) to PATH, then rebuild the library in `~/.platformio/packages/framework-arduinoespressif32-libs/esp32c5/lib/`. Copy `libnet80211.a.old` somewhere safe first, because the build step deletes it on each run:

  ```
  riscv32-esp-elf-objcopy --weaken-symbol=ieee80211_raw_frame_sanity_check libnet80211.a.old libnet80211.a
  ```
- **FastLED compile error mentioning `FP`.** Newer FastLED releases use `FP` as a type name, which clashes with Bruce's `-DFP=1`. The `.ini` pins FastLED 3.10.3, so this only appears if that pin is removed.

</details>

## Related builds

- [ESP32-S3 + 1.8" ST7735 screen and 3 buttons](https://github.com/J-Hou-ss/bruce-esp32-s3-st7735)
- [ESP32-S3 + 2.8" ILI9341 touch screen](https://github.com/J-Hou-ss/bruce-esp32-s3-ili9341-touch)

## Credits

- [Bruce](https://github.com/BruceDevices/firmware) by BruceDevices and contributors. The board files are based on Bruce's `ESP32-C5-tft` target.
- The display pins come from Waveshare's ILI9341 demo for the ESP32-C5-WIFI6-KIT.

## License

AGPL-3.0, the same as Bruce. See [LICENSE](LICENSE).
