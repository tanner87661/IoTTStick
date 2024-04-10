
# Factory Testing

To call factory test functions, please ensure that:
- NCP is initialized and is responding
- NCP is not configured (configuration is reset, the device is not connected to the cloud)

## `rpc_blynk_factoryTestWiFiAP(channel)`

Start a SoftAP on the specified WiFi channel. This can be used for the frequency test.

**Returns:** `RpcFactoryTestStatus`:
- `RPC_FACTORY_TEST_INVALID_ARGS` - call is incorrect
- `RPC_FACTORY_TEST_OK` - all is OK

## `rpc_blynk_factoryTestWiFi(ssid, pass, *rssi)`

Perform a connection to the specified WiFi 2.4 Ghz hotspot.

**Returns:** `RpcFactoryTestStatus`
- `RPC_FACTORY_TEST_INVALID_ARGS` - call is incorrect
- `RPC_FACTORY_TEST_WIFI_FAIL` - cannot connect
- `RPC_FACTORY_TEST_LOW_RSSI` - RSSI is **less than -70 dBm**
- `RPC_FACTORY_TEST_OK` - all is OK

## `rpc_blynk_factoryTestConnect()`

Test a connection to the default server. This is optional.
Should only be called after `rpc_blynk_factoryTestWiFi` reported `OK`.
This requires a good internet connection of the current hotspot.

**Returns:** `RpcFactoryTestStatus`
- `RPC_FACTORY_TEST_INET_FAIL` - cannot connect
- `RPC_FACTORY_TEST_OK` - all is OK
