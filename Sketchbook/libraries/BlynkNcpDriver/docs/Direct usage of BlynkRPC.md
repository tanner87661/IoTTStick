# Direct usage of the BlynkNcpDriver

The driver provides Blynk Cloud API that is very similar to that in [Blynk C++ API documentation](https://docs.blynk.io/en/blynk-library-firmware-api/virtual-pins).

However, because BlynkRcpDriver is based on C99, the API syntax is different:

## Fucntions

#### Blynk Cloud API 
```cpp
Blynk.begin(...)              -> rpc_blynk_initialize(tmpl_id, tmpl_name);
Blynk.virtualWrite(...)       -> rpc_blynk_virtualWrite(vpin, buffer_t)
Blynk.syncAll()               -> rpc_blynk_syncAll()
Blynk.syncVirtual(...)        -> rpc_blynk_syncVirtual(buffer_t)
Blynk.setProperty(...)        -> rpc_blynk_setProperty(vpin, property, buffer_t)
Blynk.logEvent(name)          -> rpc_blynk_logEvent(name, "")
Blynk.logEvent(name,descr)    -> rpc_blynk_logEvent(name, descr)
Blynk.resolveEvent(name)      -> rpc_blynk_resolveEvent(name)
Blynk.resolveAllEvents(name)  -> rpc_blynk_resolveAllEvents(name)
Blynk.beginGroup()            -> rpc_blynk_beginGroup(0)
Blynk.beginGroup(ts)          -> rpc_blynk_beginGroup(ts)
Blynk.endGroup()              -> rpc_blynk_endGroup()
```

#### Utility functions
```cpp
rpc_hw_getWiFiMAC(const char** mac);
rpc_hw_getEthernetMAC(const char** mac);

rpc_blynk_getNcpVersion(const char** ver);
rpc_blynk_setVendorPrefix(const char* vendor);
rpc_blynk_setVendorServer(const char* host);
rpc_blynk_setConfigTimeout(uint16_t seconds);
rpc_blynk_setConfigSkipLimit(uint8_t limit);

rpc_blynk_configReset();
```

## Callbacks

### BLYNK_CONNECTED(), BLYNK_DISCONNECTED(), Blynk.onStateChange()

```cpp
void rpc_client_blynkStateChange_impl(uint8_t state) {
    // Your handling
}
```

### BLYNK_WRITE(VPIN)

```cpp
void rpc_client_blynkVPinChange_impl(uint16_t vpin, buffer_t param)
{
    // Your handling
}
```

## `buffer_t` format

Most values will be plain strings: "Hello world", "1234", "123.456", etc.

However, sometimes the value contains multiple items (like an array).
In this case the values are separated using a `0x00` byte, i.e:

```cpp
"First\0Second\0Third"
```
