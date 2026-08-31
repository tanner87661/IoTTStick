# LILYGO TTGO T-Display porting notes

Status: planning reference only. There is not yet a supported T-Display build
target in this repository.

The board in scope is the original ESP32-based LILYGO TTGO T-Display with the
1.14-inch, 240 by 135 ST7789V display. It is not an ESP32-S3 T-Display.

## On-board hardware

| Function | ESP32 GPIO |
| --- | ---: |
| Display MOSI | 19 |
| Display SCLK | 18 |
| Display CS | 5 |
| Display DC | 16 |
| Display reset | 23 |
| Display backlight | 4 |
| I2C SDA | 21 |
| I2C SCL | 22 |
| Button 1 | 35 |
| Button 2 / boot button | 0 |

The manufacturer identifies the display as an ST7789V on a four-wire SPI bus
and recommends the Arduino `ESP32 Dev Module` target with 4 MB flash and no
PSRAM. See the [LILYGO T-Display repository](https://github.com/Xinyuan-LilyGO/TTGO-T-Display)
and [LILYGO product documentation](https://github.com/Xinyuan-LilyGO/documentation/blob/master/en/products/t-display-series/t-display/index.md).

## Button mapping required by the current firmware

`M5FunctionsStick.ino` expects three logical buttons:

| Firmware role | Current action | T-Display connection |
| --- | --- | --- |
| A | Single press: wake the screen or advance the displayed page. | Use on-board GPIO35 button. |
| B | Single press: context action, such as Wi-Fi connect or power toggle; double press: turn RedHat track power off. | Use on-board GPIO0 button. |
| C / power | Hold: prepare shutdown; three or more clicks: erase saved Wi-Fi credentials and restart. | Add an external normally-open pushbutton between GPIO27 and GND. |

The proposed GPIO27 button must be configured as `INPUT_PULLUP`; it is active
low. GPIO27 is deliberately chosen because the current sketch does not assign
it to a display, Grove, hat, or LED function. The external button should not
connect to 3.3 V.

GPIO35 is input-only, which is appropriate for the on-board A button.

GPIO0 is a boot-strapping pin. The existing on-board button is suitable for B
after boot, but it must not be held while resetting or powering the board,
otherwise the ESP32 enters its serial bootloader. A future T-Display board
profile must also move the current `hatSCL`, `hatRxD`, and `rhDataPin`
assignments away from GPIO0 before it enables the on-board B button. In the
M5Stick source those assignments currently share GPIO0 for hat communication
and RedHat LED data.

## Porting scope

Do not treat this as only a display-driver change. The sketch currently calls
M5Unified for display, button state, power management, RTC, IMU, and the
per-loop `M5.update()` operation. A T-Display target needs a small board
abstraction that:

1. Initializes the ST7789V display and GPIO4 backlight.
2. Converts GPIO35, GPIO0, and optional GPIO27 button events to the existing
   A, B, and C click/hold/double-click behavior.
3. Supplies safe no-op or optional implementations for absent M5Stick-only
   RTC, IMU, and power-management features.
4. Defines T-Display-safe replacements for the Grove and hat pin assignments.

The existing display code is oriented around the M5Stick 135 by 240 portrait
screen. The T-Display is 240 by 135; its rotation, coordinate scaling,
background images, and page layouts must be validated separately.
