# LNFP_M5Stick build, flash, and package tools

These tools build `Sketchbook/LNFP_M5Stick` with the repository's bundled
Arduino libraries. They support M5StickC Plus (`cplus`) and M5StickC Plus2
(`cplus2`) independently.

## Requirements

- Arduino CLI, on `PATH` or at `~/bin/arduino-cli`
- M5Stack ESP32 core 2.0.9 in `~/.arduino15`
- Python 3 for flashing from Linux/WSL

Core 2.0.9 is required by the repository's bundled M5GFX 0.1.15. The M5Stack
3.x core is not compatible with that M5GFX version.

## Build

Build a CPlus:

```bash
tools/LNFP_M5Stick-Linux/cplus/build.sh
```

Build a CPlus2 with the historical release layout:

```bash
tools/LNFP_M5Stick-Linux/cplus2/build.sh
```

The default output folders are `/tmp/iottstick-lnfp-cplus` and
`/tmp/iottstick-lnfp-cplus2`. Pass a different output folder as the first
argument to either script.

Each build produces:

- `LNFP_M5Stick.ino.bin` — firmware
- `LNFP_M5Stick.ino.bootloader.bin` — bootloader
- `LNFP_M5Stick.ino.partitions.bin` — partition table
- `boot_app0.bin` — boot helper
- `LNFP_M5Stick.spiffs.bin` — filesystem, configuration, and web files

Build scripts make a temporary copy of the sketch to select the correct
`StickPlus` or `StickPlus2` source definition. They do not change the source
checkout.

### Partition selection

`cplus/build.sh` always uses `no_ota`.

`cplus2/build.sh` uses `default_8MB` by default, matching the existing Plus2
release package. To use the tested no-OTA layout instead:

```bash
PLUS2_PARTITION=no_ota tools/LNFP_M5Stick-Linux/cplus2/build.sh
```

For CPlus and no-OTA CPlus2, SPIFFS is at `0x210000` and is `0x1E0000` bytes.
For default-layout CPlus2, SPIFFS is at `0x670000` and is `0x180000` bytes.

The partition choice must be the same when building and flashing. Set
`PLUS2_PARTITION=no_ota` on every CPlus2 flash command when using a no-OTA
CPlus2 build.

## Flash

Every target folder has three flash operations. They default to that target's
default build-output folder; pass another folder as the first argument.

```bash
# Update only the application. Use when the existing partition layout is
# already correct and no SPIFFS files changed.
tools/LNFP_M5Stick-Linux/cplus/flash-firmware.sh

# Update only filesystem/configuration/web files.
tools/LNFP_M5Stick-Linux/cplus/flash-spiffs.sh

# Install boot helper, bootloader, partition table, application, and SPIFFS.
tools/LNFP_M5Stick-Linux/cplus/flash-all.sh
```

The CPlus2 equivalents are in `cplus2/`. For example, a no-OTA full CPlus2
install is:

```bash
PLUS2_PARTITION=no_ota tools/LNFP_M5Stick-Linux/cplus2/flash-all.sh
```

Set `PORT` if the device is not `/dev/ttyACM0`, such as:

```bash
PORT=/dev/ttyUSB0 tools/LNFP_M5Stick-Linux/cplus/flash-firmware.sh
```

`flash-firmware.sh` writes only `LNFP_M5Stick.ino.bin` at `0x10000`; it does
not replace SPIFFS, the partition table, or saved configuration. `flash-spiffs.sh`
replaces SPIFFS, including its configuration and web files. `flash-all.sh`
replaces the complete firmware installation and SPIFFS. Each script asks for
explicit confirmation before writing.

## Create distribution ZIP files

`package-release.sh` intentionally remains above the target folders because it
builds and packages both targets in one distribution:

```bash
tools/LNFP_M5Stick-Linux/package-release.sh
```

It creates the Windows, macOS, and Linux CPlus/CPlus2 ZIP files in
`/tmp/IoTTStickM5-V<firmware-version>/InstallFile` by default. The CPlus2
archives use `default_8MB` by default; use this for no-OTA archives:

```bash
PLUS2_PARTITION=no_ota tools/LNFP_M5Stick-Linux/package-release.sh
```

The resulting Linux archives include their own `esptool.py` and `update.sh`.
They do not depend on the development tools in this directory.

## Native Linux USB access

Connect the Stick directly, then find its serial port:

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

Set `PORT` when needed, for example `PORT=/dev/ttyUSB0`. Your user must have
permission to access the serial device; on most distributions this means being
a member of the `dialout` group. Log out and back in after adding that group.

## WSL only: USB/IP attachment

The build and flash scripts above are the same under WSL. The following
Windows PowerShell commands are needed only to make a physically connected USB
device available inside WSL.

In Administrator PowerShell, bind a newly attached device once:

```powershell
usbipd bind --busid <BUSID>
```

In normal PowerShell, attach it to WSL each time:

```powershell
usbipd attach --wsl --busid <BUSID>
```

Then in WSL find the port:

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

The verified device appeared as `/dev/ttyACM0`. Detach it in PowerShell with
`usbipd detach --busid <BUSID>` when finished. None of this section applies to
a native Linux PC.
