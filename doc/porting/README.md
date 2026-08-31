# Board and display porting

This folder is the single home for design notes, pin maps, compatibility
decisions, and validation records for running `LNFP_M5Stick` on hardware other
than the M5StickC family.

It documents planned work only; no document here means that its board is a
supported firmware target.

## Current notes

- [LILYGO TTGO T-Display](lilygo-t-display.md) — original ESP32 T-Display
  hardware, button mapping, display details, and porting constraints.

## Add a board note

Each new board/display note should record:

1. Exact board revision, ESP32 variant, flash/PSRAM, and external-power model.
2. Display controller, bus, resolution, rotation, pins, and driver choice.
3. Logical button/input mapping, including any boot-strapping-pin restrictions.
4. LED, Grove/hat, LocoNet/DCC, I2C, UART, and other assigned GPIOs.
5. Available or intentionally unavailable power, battery, RTC, and IMU
   capabilities.
6. The intended Arduino core/FQBN, partition layout, dependencies, and build
   command.
7. Compile and hardware-validation results.

Keep M5Stick-specific firmware behavior unchanged unless the porting task
explicitly requires a shared abstraction or behavior change.
