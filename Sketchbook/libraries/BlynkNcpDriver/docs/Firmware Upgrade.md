# Blynk.Air

Over-The-Air (OTA) firmware updates are crucial to IoT devices because they allow for seamless software updates, security patches, and feature additions without the need for physical access to the device.

## OTA package tagging

The Blynk Cloud identifies the firmware binary by looking for a special tag embedded in it. Firmware tag contains some metadata in a form of key-value pairs. You can include such a tag in your code:

```c
#define BLYNK_PARAM_KV(k, v)    k "\0" v "\0"

volatile const char firmwareTag[] = "blnkinf\0"
    BLYNK_PARAM_KV("mcu"    , BLYNK_FIRMWARE_VERSION)       // Primary MCU: firmware version
    BLYNK_PARAM_KV("fw-type", BLYNK_FIRMWARE_TYPE)          // Firmware type (usually same as Template ID)
    BLYNK_PARAM_KV("build"  , __DATE__ " " __TIME__)        // Firmware build date and time
    BLYNK_PARAM_KV("blynk"  , BLYNK_RPC_LIB_VERSION)        // Version of the NCP driver library
    "\0";
```

> __Note__: The information inside this tag **MUST** match the information provided by the Primary MCU in runtime using the `rpc_blynk_setFirmwareInfo()`

If your OTA process involves encrypting, compressing, re-packaging (or altering the raw firmware binary in any other way), you should add the equivalent tag to your final OTA package. Blynk provides a [`blynk_tag.py`](https://github.com/blynkkk/BlynkNcpExample/blob/main/tools/blynk_tag.py) tool to automate this process.

## Firmware Update process

1. **NCP** invokes `rpc_client_otaUpdateAvailable` on the Primary MCU, indicating the `filename`, `size`, `type`, `version` of the new firmware
   - `type`, `version` could be empty strings, in case the firmware info was not detected by Blynk Cloud
2. The **Primary MCU** should validate the received info (e.g., check if it is compatible with the current hardware and software) and return true if it accepts the update
3. If the **Primary MCU** confirms the update (returns OK), it **may** perform any of the following steps (as needed):
   - Wait until the firmware update is appropriate, and finish any critical operations
   - Re-configure the NCP UART, i.e. change the baud rate
   - Reboot into bootloader mode or load a special firmware for handling the update process
   - Initialize the memory for receiving the new firmware (e.g., erase the target memory sector)
   - Ping NCP several times to ensure that the communication is working and the framing layer is synchronised
   - If the ping-pong process fails, the Primary MCU can attempt to reinitialize the communication (e.g., reconfigure the UART) and retry the process
   - etc.
4. **Primary MCU** invokes `rpc_blynk_otaUpdateStart` function, indicating a suggested chunk size
   - The **NCP** should validate the proposed chunk size and ensure it is within acceptable limits
5. **NCP** invokes `rpc_client_otaUpdateWrite` for each chunk sequentially; each request includes `offset`, `data`, `CRC32`
   - The actual chunk size can vary throughout the update, but it should never exceed the suggested chunk size
   - The **Primary MCU** returns false if it encounters a CRC mismatch or otherwise cannot process the packet
   - If the request fails or times out, **NCP** should resend the request up to 5 times before terminating the OTA process
   - If **NCP** faces issues downloading the update (i.e. due to the network failure), it invokes `rpc_client_otaUpdateCancel`
   - **Primary MCU** should also cancel the OTA if:
     - NCP stops sending new chunks of data (i.e. after 1 minute)
     - There is a power failure
6. After the last packet is transfered, **NCP** invokes `rpc_client_otaUpdateFinish`
   - The **Primary MCU** can query additional info about the firmware, i.e. call `CRC32`, `MD5` or `SHA256` digest of the complete file
   - The **Primary MCU** should perform a final verification of the received firmware before confirming the completion of the update process
   - The **Primary MCU** returns `True` to the NCP to indicate that OTA will be applied.
   - **NCP** reboots at this stage and waits initialization from the Primary MCU (this step may be unneded/modified in future)
7. The **Primary MCU** reboots and loads the new firmware
8. As part of it's regular initialization procedure, the **Primary MCU** sends the new firmware type and version to the NCP
9. The **NCP** provides the new firmware info to the cloud, at which point the upgrade is succesfully completed

Here's the [reference implementation of this process](https://github.com/blynkkk/blynk-library/blob/master/src/utility/BlynkNcpOtaImpl.h).

# Resilent OTA upgrade

The OTA update mechanism must be reliable, secure, and efficient to prevent the risk of bricking the device or leaving it open to security breaches. Below are some conditions that should be considered:

- **Insufficient Power**: if the device is running on a battery, the MCU should check the battery level before calling `rpc_blynk_otaUpdateStart`. If there's not enough power to complete the update, it should delay the process until there's sufficient power.
- **Insufficient Flash Storage**: MCU should also check if there's enough storage space to download and apply the update.
- **Corrupted Firmware File**: MCU should validate the downloaded firmware, typically through checksums or cryptographic signatures, to ensure it hasn't been corrupted during the download. If corruption is detected, the MCU should discard the corrupted firmware and start a new download.
- **New Firmware Malfunction**: the firmware update may be successful from an installation perspective, but the new firmware version may malfunction/crash due to various reasons such as programming errors, unhandled exceptions, or device-specific issues. To handle this, the MCU should include a post-update self-diagnostics phase. If this diagnostic test fails, the MCU should ideally rollback to the previous firmware version.
- **Generic Update Failure** (Device Reset, Power Loss, Unstable Network, etc.): the MCU should be able to recover and either continue the update or roll back to the previous version on the next boot. Bricking of the device must be avoided.

It is recommended to emloy one of these OTA update strategies:

## Dual-Bank (A/B) OTA Updates

In this (**recommended**) method, the flash memory of the MCU is divided into two separate sections (partitions, banks). One bank is used to run the existing firmware (Active), while the other is used to download and store the new firmware (Standby). After a successful update, the system reboots and switches to using the updated firmware. This method provides a fallback mechanism: if the update fails or the new firmware is faulty, the system can boot using the old, reliable firmware.

- **Pros:** Provides high reliability, allows for immediate rollback if the update fails or new firmware is faulty.
- **Cons:** Requires twice the memory to store two copies of the main firmware.

## NCP-assisted fail-safe OTA Updates

In this alternative method, `Blynk.NCP` is used to assist the primary MCU during the update process. The primary MCU's bootloader fetches the firmware update from the NCP, verifies it, and applies it. If the update fails the bootloader can retry the update, mitigating the risk of bricking the device.

The internal filesystem of the NCP can typically store the complete MCU firmware binary. Before invoking `rpc_blynk_otaUpdateStart`, the MCU should call the `rpc_blynk_otaUpdatePrefetch` function. Firmware pre-fetching is recommended to eliminate the dependence on a stable network connection while the device is in bootloader mode.

- **Pros:** Requires only one flash partition allocated for the main firmware. The bootloader can retry the update as many times as needed.
- **Cons:** Rollback to a previous version of firmware is difficult to implement if the update fails or new firmware is faulty. The device cannot operate normally until the OTA update is completed successfully.

---

Irrespective of the method you choose, testing the OTA firmware updates under various conditions is crucial before deploying in a production environment. It will ensure that your IoT devices function as expected even after the firmware updates.
