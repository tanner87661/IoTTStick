# Distribution template snapshot

This directory snapshots the installer inputs previously kept only inside the generated release archives.

- `windows/`: `update.bat` and its required `esptool.exe`, copied from the V1.6.9 Windows installer.
- `macos/`: `update.mac` and `esptool.py`, copied from the V1.6.9 macOS installer.
- `linux/`: the current self-contained Linux updater template; package builds place `esptool.py` beside it.
- `Readme.txt` and `MacReadMe.txt`: the V1.6.9 installer notes.

`package-release.sh` uses the bundled Windows executable and Python uploader rather than referencing `IoTTStickM5 Install Files/`. It generates target-specific updater scripts with the partition offsets selected for the new build; the scripts here are retained as provenance and starting templates.
