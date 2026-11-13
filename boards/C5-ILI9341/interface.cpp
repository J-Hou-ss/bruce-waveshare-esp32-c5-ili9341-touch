// Bruce board interface: Waveshare ESP32-C5-WIFI6-KIT + 2.8" ILI9341 screen with XPT2046 touch.
// Touch only, no buttons. Pins and wiring are in pins_arduino.h in this folder.
// Based on Bruce's boards/ESP32-C5-tft/interface.cpp (BruceDevices, AGPL-3.0).
//
// What differs from the stock C5 TFT board:
//   - pins moved off GPIO6 (clock) and the screen RESET driven from a GPIO (see pins_arduino.h)
//   - every chip select on the shared SPI bus is parked HIGH before the screen starts
//   - touch calibration is checked before it is saved, and can be redone without a PC
//   - three full-height touch zones (previous / select / next) plus a back corner
#include "core/powerSave.h"
#include "core/utils.h"
#include <interface.h>

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    // One SPI bus is shared by the screen, the touch chip, the SD card and the radio modules.
    // Deselect all of them first so nothing answers while the screen is being set up.
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);
    pinMode(SDCARD_CS, OUTPUT);
    digitalWrite(SDCARD_CS, HIGH);
    pinMode(CC1101_SS_PIN, OUTPUT); // CC1101, NRF24 and W5500 share one pin on this board
    digitalWrite(CC1101_SS_PIN, HIGH);
    pinMode(NRF24_SS_PIN, OUTPUT);
    digitalWrite(NRF24_SS_PIN, HIGH);
    pinMode(W5500_SS_PIN, OUTPUT);
    digitalWrite(W5500_SS_PIN, HIGH);

    pinMode(TFT_MOSI, OUTPUT);
    digitalWrite(TFT_MOSI, HIGH);
    pinMode(TFT_SCLK, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    digitalWrite(TFT_DC, HIGH);
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH); // the display library pulses this low when it starts
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    bruceConfig.colorInverted = 0; // plain (non-IPS) ILI9341
}

/*********************************************************************
** Touch calibration
** Bruce keeps the five TFT_eSPI calibration numbers in /calData on LittleFS.
** [0],[2] = raw X/Y of one edge, [1],[3] = raw X/Y span, [4] = rotate/invert flags.
**********************************************************************/
#define CAL_FILE "/calData"
#define CAL_MIN_SPAN 2000  // a real four-corner calibration spans about 2800-3700 of the 4096 raw steps
#define TOUCH_HELD_Z 600   // raw pressure that counts as a firm press
#define RECAL_HOLD_MS 1500 // finger held this long at start-up = redo the calibration

// A calibration made from stray readings (touch wires loose, or the screen tapped in the wrong places)
// has a narrow span, and then only one corner of the screen responds. Never keep one like that.
static bool calLooksValid(const uint16_t *c) { return c[1] >= CAL_MIN_SPAN && c[3] >= CAL_MIN_SPAN && c[4] <= 7; }

static bool loadCalibration(uint16_t *c) {
    File f = LittleFS.open(CAL_FILE, "r");
    if (!f) return false;
    for (int i = 0; i < 5; i++) c[i] = f.readStringUntil('\n').toInt();
    f.close();
    Serial.printf("\ntft Calibration data: %d, %d, %d, %d, %d\n", c[0], c[1], c[2], c[3], c[4]);
    return calLooksValid(c);
}

static void saveCalibration(const uint16_t *c) {
    File f = LittleFS.open(CAL_FILE, "w");
    if (!f) return;
    f.printf("%d\n%d\n%d\n%d\n%d\n", c[0], c[1], c[2], c[3], c[4]);
    f.close();
}

static void calMessage(const char *line1, const char *line2) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawCentreString(line1, tft.width() / 2, tft.height() / 2 - 20, 1);
    tft.drawCentreString(line2, tft.width() / 2, tft.height() / 2 + 6, 1);
}

// True while the screen is pressed firmly. Used before our own calibration is loaded, so the
// display library is still on its built-in default calibration: any press away from the very
// edge counts, which is all that is needed here.
static bool screenPressed() {
    uint16_t x, y;
    return tft.getTouch(&x, &y, TOUCH_HELD_Z);
}

// True if the screen stays pressed for the whole of holdMs.
static bool touchHeldFor(uint32_t holdMs) {
    uint32_t start = millis();
    while (millis() - start < holdMs) {
        if (!screenPressed()) return false;
        delay(20);
    }
    return true;
}

static void runCalibration(uint16_t *c) {
    tft.setRotation(ROTATION); // always calibrate in the default rotation; InputHandler handles the others
    for (;;) {
        calMessage("Touch calibration", "Tap each corner arrow");
        tft.calibrateTouch(c, TFT_WHITE, TFT_BLACK, 10);
        if (calLooksValid(c)) break;
        Serial.printf("\nCalibration rejected: %d, %d, %d, %d, %d\n", c[0], c[1], c[2], c[3], c[4]);
        calMessage("That did not work", "Tap right on the arrows");
        delay(2000);
    }
    saveCalibration(c);
    tft.setRotation(bruceConfigPins.rotation);
    tft.fillScreen(TFT_BLACK);
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup: load the touch calibration, or make one
***************************************************************************************/
void _post_setup_gpio() {
    uint16_t calData[5];
    bool haveCal = loadCalibration(calData);

    // Way out of a bad calibration without a PC: keep a finger on the middle of the screen while the board starts.
    if (haveCal && touchHeldFor(RECAL_HOLD_MS)) {
        calMessage("Recalibrate touch", "Lift your finger");
        while (screenPressed()) delay(20);
        delay(500);
        haveCal = false;
    }

    if (!haveCal) runCalibration(calData);
    tft.setTouch(calData);
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { return 0; }

/***************************************************************************************
** Function name: isCharging()
** Description:   Default implementation that returns false
***************************************************************************************/
bool isCharging() { return false; }

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    if (brightval == 0) {
        analogWrite(TFT_BL, brightval);
    } else {
        int bl = MINBRIGHT + round(((255 - MINBRIGHT) * brightval / 100));
        analogWrite(TFT_BL, bl);
    }
}

/*********************************************************************
** Touch zones (replaces Bruce's 3x3 touchHeatMap for this board)
**   [x] corner, top-left        -> Esc / back
**   left  40% of the width      -> Prev  (up / left)
**   middle 20%                  -> Select
**   right 40%                   -> Next  (down / right)
** The columns run the full height, footer included. touchPoint is still filled in,
** so screens that read the touch position themselves (the keyboard) keep working.
**********************************************************************/
#define TOUCH_ESC_W 64     // px from the left edge: size of the [x] hit area
#define TOUCH_ESC_H 48     // px from the top edge
#define TOUCH_LEFT_PCT 40  // left zone ends here (% of width)
#define TOUCH_RIGHT_PCT 60 // right zone starts here (% of width)

// A side tap raises two flags (Prev+Up or Next+Down), because some Bruce screens listen for
// Prev/Next and others for Up/Down. Only one of the two may be acted on, or menus move two steps.
static bool prevPairPending = false;
static bool nextPairPending = false;

// Bruce's check() calls InputHandler() just before it tests a flag (USE_TFT_eSPI_TOUCH builds).
// So as soon as a screen has used one flag of a pair, drop the other one here.
// This keeps the whole behaviour inside the board folder: no change to Bruce's shared code.
static void settleTouchPairs() {
    if (prevPairPending && !(PrevPress && UpPress)) {
        PrevPress = false;
        UpPress = false;
        prevPairPending = false;
    }
    if (nextPairPending && !(NextPress && DownPress)) {
        NextPress = false;
        DownPress = false;
        nextPairPending = false;
    }
}

static void touchZones(int x, int y) {
    if (x < TOUCH_ESC_W && y < TOUCH_ESC_H) {
        EscPress = true;
        return;
    }
    const int leftEdge = tftWidth * TOUCH_LEFT_PCT / 100;
    const int rightEdge = tftWidth * TOUCH_RIGHT_PCT / 100;
    if (x < leftEdge) {
        PrevPress = true;
        UpPress = true;
        prevPairPending = true;
    } else if (x < rightEdge) {
        SelPress = true;
    } else {
        NextPress = true;
        DownPress = true;
        nextPairPending = true;
    }
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static unsigned long tm = 0;
    settleTouchPairs();
    if (millis() - tm < 200 && !LongPress) return;

    TouchPoint t;
    checkPowerSaveTime();
    if (!tft.getTouch(&t.x, &t.y)) return;

    NextPress = false;
    PrevPress = false;
    UpPress = false;
    DownPress = false;
    SelPress = false;
    EscPress = false;
    AnyKeyPress = false;
    NextPagePress = false;
    PrevPagePress = false;
    prevPairPending = false;
    nextPairPending = false;
    touchPoint.pressed = false;

    // The calibration is made in ROTATION (1). Turn the point for the other three rotations.
    if (bruceConfigPins.rotation == 3) {
        t.y = (tftHeight + 20) - t.y;
        t.x = tftWidth - t.x;
    }
    if (bruceConfigPins.rotation == 0) {
        uint16_t tmp = t.x;
        t.x = map((tftHeight + 20) - t.y, 0, 320, 0, 240);
        t.y = map(tmp, 0, 240, 0, 320);
    }
    if (bruceConfigPins.rotation == 2) {
        uint16_t tmp = t.x;
        t.x = map(t.y, 0, 320, 0, 240);
        t.y = map(tftWidth - tmp, 0, 240, 0, 320);
    }
    Serial.printf("\nTouch x=%d, y=%d, rot=%d\n", t.x, t.y, bruceConfigPins.rotation);

    if (!wakeUpScreen()) AnyKeyPress = true;
    else return;

    touchPoint.x = t.x;
    touchPoint.y = t.y;
    touchPoint.pressed = true;
    touchZones(t.x, t.y);
    tm = millis();
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() {}

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to turnoff the device (name is odd btw)
**********************************************************************/
void checkReboot() {}
