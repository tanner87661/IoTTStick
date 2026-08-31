# IoTTStick Repository Guide

## Project focus

IoTTStick is an Arduino/ESP32 firmware repository for the IoTT Stick and related model-railroad communication modules and hat devices. The primary, actively maintained application is `Sketchbook/LNFP_M5Stick`.

Treat the other directories under `Sketchbook/` as examples, experiments, hardware tests, companion-device firmware, or older projects unless a task explicitly names one of them. Do not make broad changes to those sketches merely to keep them in sync with `LNFP_M5Stick`.

## Primary firmware

- Main sketch: `Sketchbook/LNFP_M5Stick/LNFP_M5Stick.ino`
- Supporting `.ino` tabs in that directory are part of the same Arduino sketch and are compiled together.
- Board/display porting notes belong in `Sketchbook/LNFP_M5Stick/porting/`. These are planning and validation records; they do not by themselves add a supported board target.
- The intended hardware is the M5StickC family, including StickC/StickC Plus targets. The source also contains support for StickC Plus2; check the `StickPlus` and `StickPlus2` preprocessor definitions at the top of the main sketch before building or changing hardware-specific code.
- Preserve target-specific pin assignments, power handling, memory/partition requirements, and conditional compilation. A change that works on one Stick model must not silently break another.
- Web assets and runtime files associated with the sketch belong under `Sketchbook/LNFP_M5Stick/data` when applicable.

## Repository layout

- `Sketchbook/LNFP_M5Stick/`: main IoTT Stick firmware.
- `Sketchbook/libraries/`: the Arduino libraries currently used to provide a reproducible local development environment, including the IoTT-specific libraries.
- Other `Sketchbook/*/` directories: sample sketches, tests, experiments, and firmware for related boards or accessories.
- `Communication Modules/`, `Hat Devices/`, and `Enclosures/`: hardware-related project material.
- `ConfigData/`: configuration-related project data.
- `IoTTStickM5 Install Files/`: packaged release/install artifacts. Do not edit generated release archives as a substitute for changing source.
- `README.md`: release history and high-level repository information.

## Dependencies

For now, assume builds use this repository as an Arduino sketchbook so libraries resolve from `Sketchbook/libraries`. Some dependencies are standard Arduino/ESP32 or M5Stack libraries, while the `IoTT_*` libraries contain project-specific functionality.

`LNFP_M5Stick.ino` documents additional constraints, such as the bundled ArduinoJson 7.0.4. Verify compatibility before upgrading any library; do not automatically replace the bundled versions or modify library code as part of an unrelated firmware change.

The bundled M5GFX 0.1.15 is compatible with the M5Stack ESP32 2.0.x platform, but not M5Stack ESP32 3.3.8. Use M5Stack ESP32 2.0.9 for the current bundled dependency set.

A future goal is to manage dependencies outside this repository and move to newer, explicitly versioned releases where compatible. Changes toward that goal should be handled as a dedicated migration: add a reproducible dependency manifest/build process, test all supported M5Stick targets, and remove bundled libraries only after the replacement build is verified.

## Development guidelines

- Keep changes scoped to the sketch or hardware named by the task.
- Search for shared types and behavior in `Sketchbook/libraries/IoTT_*` before changing interfaces in the main sketch.
- Preserve Arduino's multi-tab sketch model: declarations and globals may be shared across the `.ino` files even when no local header is visible.
- Avoid committing IDE state, local board configuration, credentials, Wi-Fi settings, or device-specific configuration.
- Keep user-facing version strings and release notes consistent when preparing an actual release; do not bump versions for ordinary development changes unless requested.
- Prefer small, reviewable changes. Avoid mass-formatting legacy sketches or bundled third-party libraries.

## Validation

There is no repository-wide automated test suite. Validate changes with the narrowest relevant checks available:

1. Compile `Sketchbook/LNFP_M5Stick` with the intended ESP32/M5Stick board selection and the repository's Arduino libraries.
2. Compile once per affected hardware variant when changing pins, display, power, partitioning, or target conditionals.
3. Upload to representative hardware and smoke-test startup, display/buttons, Wi-Fi/web UI, and the affected LocoNet, DCC, MQTT, or hat-device path when hardware is available.
4. If hardware validation is not possible, state exactly which compile or static checks were completed and what remains unverified.

Do not claim full validation based only on compiling one target when a change affects multiple board variants.

Arduino CLI starting points for the original M5Stick-C board and the currently selected `StickPlus2` source configuration, tested with M5Stack ESP32 2.0.9, are:

```bash
arduino-cli compile --fqbn m5stack:esp32:stickc --libraries Sketchbook/libraries Sketchbook/LNFP_M5Stick
arduino-cli compile --fqbn m5stack:esp32:stickc_plus2:PartitionScheme=no_ota --libraries Sketchbook/libraries Sketchbook/LNFP_M5Stick
```

For StickC Plus2, `PartitionScheme=no_ota` is required: it provides a 2 MiB application partition and a `0x1E0000`-byte (1.875 MiB) SPIFFS partition. The Arduino menu labels this option “2MB APP/2MB SPIFFS.” The default Plus2 partition layout (3 MB application / 1.5 MB SPIFFS) can compile successfully but prevents the firmware from reliably loading its configuration and Wi-Fi settings at runtime. The firmware upload does not include the sketch's `data/` directory; generate and flash its SPIFFS image separately. See `tools/LNFP_M5Stick-Linux/` for the Linux tooling and WSL-specific USB/IP notes.
