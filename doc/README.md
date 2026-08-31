# IoTTStick documentation

IoTTStick is firmware and hardware material for the IoTT Stick ecosystem of
model-railroad communication modules and hat devices. It supports functions
including LocoNet, DCC, MQTT, WiThrottle, OpenLCB, web configuration, and
IoTT hat hardware.

## Primary firmware

The actively maintained application is
[`Sketchbook/LNFP_M5Stick`](../Sketchbook/LNFP_M5Stick/). Its main sketch is
[`LNFP_M5Stick.ino`](../Sketchbook/LNFP_M5Stick/LNFP_M5Stick.ino); all `.ino`
files in that directory are Arduino sketch tabs compiled together.

The intended hardware is the M5StickC family (StickC and StickC Plus), with
source conditionals for StickC Plus2. Check the `StickPlus` and `StickPlus2`
definitions near the top of the main sketch before compiling or changing
hardware-specific code. Pin assignments, power behavior, and memory layouts
are target-specific.

## Build requirements

Use this repository as the Arduino sketchbook so the bundled dependencies in
`Sketchbook/libraries` are found. The current dependency set is compatible
with M5Stack ESP32 core 2.0.9 and M5GFX 0.1.15; it is not compatible with
M5Stack ESP32 core 3.3.8. ArduinoJson must remain on the bundled version,
7.0.4.

Example Arduino CLI builds:

```bash
# With StickPlus enabled and StickPlus2 disabled in LNFP_M5Stick.ino:
arduino-cli compile --fqbn m5stack:esp32:stickc_plus:PartitionScheme=no_ota --libraries Sketchbook/libraries Sketchbook/LNFP_M5Stick

# With StickPlus2 enabled and StickPlus disabled (the default source selection):
arduino-cli compile --fqbn m5stack:esp32:stickc_plus2:PartitionScheme=default_8MB --libraries Sketchbook/libraries Sketchbook/LNFP_M5Stick
```

For StickC Plus, use `no_ota` (Arduino menu label: `2MB APP/2MB SPIFFS`).
StickC Plus2 uses `default_8MB` by default (Arduino menu label: `8M with
spiffs (3MB APP/1.5MB SPIFFS)`). If you use the Plus2 `no_ota` alternative,
build and flash the firmware, partition table, and SPIFFS image with that same
layout.

The firmware upload does not include the sketch's `data/` directory. Generate
and flash its SPIFFS image separately; Linux tooling and WSL USB/IP notes are
in [`tools/LNFP_M5Stick-Linux`](../tools/LNFP_M5Stick-Linux/).

For a VS Code setup using Arduino Maker Workshop, including the separate
StickPlus and StickPlus2 board and partition selections, see
[VS Code with Arduino Maker Workshop](vscode-arduino-maker-workshop.md).

## Repository guide

| Location | Purpose |
| --- | --- |
| `Sketchbook/LNFP_M5Stick/` | Main IoTT Stick firmware and its web/runtime data. |
| `Sketchbook/libraries/` | Bundled Arduino and IoTT libraries used for reproducible builds. |
| `doc/porting/` | Planning and validation notes for potential board/display ports. |
| `Communication Modules/`, `Hat Devices/`, `Enclosures/` | Hardware design material. |
| `ConfigData/` | Configuration-related project data. |
| `IoTTStickM5 Install Files/` | Packaged release and install artifacts. |
| `tools/` | Development, packaging, and deployment tools. |

Other sketches under `Sketchbook/` are examples, experiments, tests, or
companion-device firmware unless a task names them explicitly.

## Porting documentation

Board/display notes are planning records, not declarations of supported
targets. See [the porting index](porting/README.md) before starting a new
board port. A supported port requires the relevant compile checks and, where
possible, hardware validation of startup, display/buttons, Wi-Fi/web UI, and
affected communication or hat paths.

## Release history

The repository's high-level release history is maintained in the
[top-level README](../README.md). Packaged install artifacts are historical
release outputs; change firmware source rather than editing those archives.
