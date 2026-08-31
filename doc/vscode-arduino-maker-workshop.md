# VS Code with Arduino Maker Workshop

This guide configures Visual Studio Code to build and upload the primary
IoTTStick firmware, [`Sketchbook/LNFP_M5Stick`](../Sketchbook/LNFP_M5Stick/),
with the **Arduino Maker Workshop** extension. It is an Arduino CLI workflow;
the sketch is not a PlatformIO project.

## Prerequisites

Install the following before opening the sketch:

- A current stable [Visual Studio Code](https://code.visualstudio.com/) desktop
  release.
- The **Arduino Maker Workshop** VS Code extension
  (`TheLastOutpostWorkshop.arduino-maker-workshop`), version 1.1.5 or later.
  Its bundled Arduino CLI is sufficient; do not configure a second CLI unless
  there is a specific reason.
- A current Microsoft **C/C++** extension (`ms-vscode.cpptools`) for
  IntelliSense.
- The **M5Stack ESP32 board package, version 2.0.9**. Do not use 3.x for this
  repository: the bundled M5GFX 0.1.15 is compatible with the 2.0.x core, not
  M5Stack ESP32 3.x.

The repository already bundles the sketch dependencies in
`Sketchbook/libraries`. Do not use Library Manager to install replacement
copies of those libraries. ArduinoJson must remain on the bundled version,
7.0.4.

## One-time extension setup

1. Open **File > Open Folder** and choose
   `Sketchbook/LNFP_M5Stick`. Arduino Maker Workshop uses the first workspace
   folder as the active sketch.
2. Open **Arduino Maker Workshop Home** from the VS Code Activity Bar.
3. In the extension settings, set **User Directory** (the Arduino sketchbook
   directory) to the repository's `Sketchbook` directory. For example:

   ```text
   /path/to/IoTTStick/Sketchbook
   ```

   This is what makes `Sketchbook/libraries` available to the build.
4. In **Boards Manager**, add the M5Stack Board Manager URL:

   ```text
   https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json
   ```

5. Install **M5Stack ESP32 2.0.9**. If another version is already installed,
   use Boards Manager to install and select 2.0.9 for this project.

After choosing a board and compiling successfully, Maker Workshop generates
the C/C++ IntelliSense configuration. If IntelliSense shows stale errors after
changing board settings, compile again.

## Select the firmware target

The top of `LNFP_M5Stick.ino` selects the firmware's hardware behavior. Only
one target definition may be enabled, and it must match the selected Arduino
board.

### M5StickC Plus (StickPlus)

Use this source selection:

```cpp
#define  StickPlus
//#define  StickPlus2
```

In **Board Selection**, select **M5StickC Plus**. In **Board Configuration**,
select:

```text
No OTA (2MB APP/2MB SPIFFS)
```

The corresponding Arduino board option is `PartitionScheme=no_ota`.

### M5StickC Plus2 (StickPlus2)

Use this source selection:

```cpp
//#define  StickPlus
#define  StickPlus2
```

In **Board Selection**, select **M5StickC Plus2**. For the standard historical
Plus2 release layout, use this **Board Configuration** value:

```text
8M with spiffs (3MB APP/1.5MB SPIFFS)
```

It maps to `PartitionScheme=default_8MB` and is the Plus2 board's default.

`No OTA (2MB APP/2MB SPIFFS)` (`PartitionScheme=no_ota`) is also a supported
Plus2 alternative. Use it only when the firmware, SPIFFS image, and flashing
commands all use the same no-OTA layout. The repository tools describe both
layouts in [their README](../tools/LNFP_M5Stick-Linux/README.md#partition-selection).

## Build and upload

1. Confirm the source target, board, and partition setting match one of the
   sections above.
2. Click **Compile** in Arduino Maker Workshop. The extension writes generated
   build output under the sketch's `build/` directory; it is intentionally
   ignored by Git.
3. Connect the Stick over USB, select its serial port in the extension, and
   click **Upload**.

Uploading the sketch alone does not flash `Sketchbook/LNFP_M5Stick/data`.
That directory supplies SPIFFS content such as configuration, Wi-Fi settings,
and web assets. Generate and flash the matching SPIFFS image separately when
needed. On Linux/WSL, use the target-specific scripts under
`tools/LNFP_M5Stick-Linux/`; a full installation writes the partition table,
firmware, and SPIFFS together.

## Keep partition layouts consistent

Changing a partition scheme changes where SPIFFS lives in flash. Do not flash
a firmware built for one layout onto a device whose SPIFFS image or partition
table came from the other. When changing between StickPlus and StickPlus2, or
between Plus2 layouts, use the appropriate full flash procedure so all parts
of the installation agree.
