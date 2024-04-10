# Time and Location

## `rpc_blynk_setTime(Int64 time)`

- `time`: The current time in milliseconds elapsed since the UNIX epoch (`00:00:00 UTC on 1 January 1970`)

Initialize or update the NCP time from an external time source like an RTC, GPS/GNSS receiver, etc.
This function should be called during the NCP initialization, and allows `rpc_blynk_getTime` to provide the local time information before the device gets online.
Should also be called by the MCU to adjust the NCP clock during prolonged offline periods (when an `RPC_EVENT_BLYNK_TIME_SYNC` event is received).

## `rpc_blynk_getTime(String iso8601, Int64 time, Int16 offset, String tz_abbr, UInt8 dst_status)`

- `iso8601`: An ISO 8601 formatted date string
- `time`: The current time in milliseconds elapsed since the UNIX epoch
- `offset`: The local time offset in minutes, including Daylight Saving Time (DST).  
  Negative offsets are **west** of UTC, positive offsets are **east** of UTC
- `tz_abbr`: Short timezone abbreviation
- `dst_status`: The DST status
  - **0 - No information**: The system doesn't have sufficient information about the timezone or DST status. This usually indicates that the device was never connected to the cloud, or the device configuration was reset
  - **1 - Not applicable**: The current timezone does not observe DST. This is common in many regions worldwide, particularly near the equator, where daylight hours don't vary much throughout the year
  - **2 - Active**: The system is currently observing DST. In regions that follow DST, this typically means clocks are adjusted forward by one hour
  - **3 - Inactive**: The system is **not** currently observing DST. In regions that follow DST, this usually means clocks are set to their standard time

This function retrieves the current time and related details from the NCP.  

Returns `false` if time information is incomplete (i.e. neither available from the cloud yet, nor configured by `rpc_blynk_setTime`).

## `rpc_blynk_getLocationInfo(String lat, String lon, String olson_id, String posix_tz)`

- `lat`: The approximate GPS latitude
- `lon`: The approximate GPS longitude
- `olson_id`: The OLSON timezone identifier
- `posix_tz`: The POSIX timezone rule

This function retrieves the location information stored in the NCP's non-volatile memory (NVM). `lat` and `lon` can be used to implement astronomical events like sunrise and sunset.
**Note:** This information is available even when the device is offline, reducing the necessity for the MCU to retain this information.

Returns `false` if device was not connected to the cloud yet (i.e. before the initial provisioning or after the NCP configuration reset).

## Events

`rpc_client_processEvent` will be called by NCP under certain conditions:

- `RPC_EVENT_BLYNK_TIME_SYNC` - NCP sends this event to request time adjustment during prolonged offline periods. MCU should call `rpc_blynk_setTime`
- `RPC_EVENT_BLYNK_TIME_CHANGED` - Time or Location changed. MCU should call `rpc_blynk_getTime` and `rpc_blynk_getLocationInfo` to get the details. Possible reasons for this event:
  - Time provided by Blynk cloud is more than 30 seconds away from the time provided by `rpc_blynk_setTime`
  - Location or timezone changed (NCP refreshes this info on each connection or every 6 hours)
  - DST starts or ends

## Battery backed RTC

The RTC must be configured to store the time internally **in UTC**.
During startup initialization sequence, the Primary MCU should check if the RTC is configured and running.
If RTC time is meaningful, the MCU should call `rpc_blynk_setTime()` to adjust the NCP clock.
After this, MCU can call `rpc_blynk_getTime` and `rpc_blynk_getLocationInfo` to adjust it's own local time.
