# `Blynk.NCP` protocol specification

```
Version: 20230402
Author:  Volodymyr Shymanskyy
Status:  draft
```

## Message format

- **`little-endian`** byte order is used
- each message starts with a _magic number_ that defines an operation type
- messages with an unknown opcode should be ignored
- operations are symmetrical, i.e. they can be issued (and should be handled) by both `MCU` and the `NCP`
- such operation types are defined currently: `INVOKE`, `RESULT`, `ONEWAY`

### `INVOKE`

Invoke a remote procedure.

- `uint16_t` Opcode = `0x67BC`
- `uint16_t` RpcUID: unique identifier of a function to call
- `uint16_t` MsgID: unique identifier of a request
- `<buffer>` Args: function-specific arguments

### `RESULT`

Return status and results of a remote procedure call.

- `uint16_t` Opcode = `0xA512`
- `uint16_t` MsgID: unique identifier of a request (should match `MsgID` in the associated `INVOKE`)
- `uint8_t`  Status: the status of the invocation
- `<buffer>` Rets: function-specific return values

Status codes: `0x00` - Success, `0x10` - Generic error, `0x11` - Timeout, `0x12` - Not enough memory, `0x13` - Unknown RpcUID, `0x14` - Args data missmatch, `0x15` - Results encoding failed

### `ONEWAY`

Same as `INVOKE`, but the corresponding `RESULT` message is not sent or expected to be received.

- `uint16_t` Opcode = `0x1CF3`
- `uint16_t` RpcUID: unique identifier of a function to call
- `<buffer>` Args: function-specific arguments

## UART transport layer

The initial serial port configuraton is `38400 8N1` (or `115200` in some variants),  flow control: `none`.

```
 ┌───────────────┐        ┌──────────────────┐
 │            RX │<───────│ TX               │
 │   MCU      TX │───────>│ RX   Blynk.NCP   │
 │           GND │────────│ GND              │
 └───────────────┘        └──────────────────┘
```

A symmetrical framing protocol is used to transfer binary messages over `UART`. The purpose of framing is:

 - Transmitting datagrams of a known size over a serial line
 - Enable using `Software Flow Control` while the binary data is transferred
 - Help skipping the non-meaningful data (such as serial line garbage or the MCU boot messages)
 - Re-synchronizing after failures such as data corruption

### In-band signaling

A variation of the `SLIP` protocol is used. Serial data is split into packets. Each packet starts with `BEG(0xAA)` and ends with `END(0xBB)` byte. To allow using `Software Flow Control`, the `XON(0x11)` and `XOFF(0x13)` are treated as reserved bytes (not used for the data transmission).

### Payload data escaping

Because the payload is binary and may include any `8-bit` symbols, the `ESC(0xCC)` marker is used for escaping.
The transmitter **MUST** transform `0x11`, `0x13`, `0xAA`, `0xBB`, `0xCC` bytes in the payload to `ESC x^0xFF` form, i.e.:

```
  XON  => ESC 0xEE
  XOFF => ESC 0xEC
  BEG  => ESC 0x55
  END  => ESC 0x44
  ESC  => ESC 0x33
```

The receiving end **MUST** restore (unescape) such sequences to get the original payload.

### Frame structure

```
BEG <escaped payload> <escaped CRC> END
```

- `CRC` is a standard `CRC-8-CCITT` checksum of the **raw/unescaped** payload data
- any data after `END` and before `BEG` should be discarded
- receiver should be ready to receive at least `1536` bytes of raw (unescaped) data
- sender should **not** send more than `1536` bytes of raw (unescaped) data, unless a bigger buffer is negotiated between the endpoints

## `Blynk.NCP` interfaces

- The available interfaces along with their methods are described in the [IDL file](../BlynkNcpClient/extra/BlynkNCP.jidl)
- `Blynk.NCP` hosts such interfaces: `system`, `uart`, `blynk`
- `NCP client` hosts such interfaces: `system`, `client`
- The corresponding `RpcUID`s are defined in [BlynkRPC.h](../BlynkNcpClient/lib/BlynkRPC/src/BlynkRpc.h)

## NCP Initialization

#### Ping the device
```log
MCU -> NCP
<< aa   bc 67   01 01  02 00  70   bb
   |    |       |      |      |    |
   BEG  invoke  func   msgId  CRC  END

NCP -> MCU
>> aa   12 a5   02 00  00     91   bb
   |    |       |      |      |    |
   BEG  result  msgId  status CRC  END
```

#### Configure the product

Set `TemplateID = TMPL0vFkFmJM7` and `TemplateName = QPower`

```log
<< aa bc 67 01 03 06 00 54 4d 50 4c 30 76 46 6b 46 6d 4a 4d 37 00 51 50 6f 77 65 72 00 68 bb
>> aa 12 a5 06 00 00 01 a1 bb
```

The `Blynk.NCP` should start.

## NCP Reinitialization

When NCP starts, it enters the `NOT_INITIALIZED` state and won't connect to the Cloud until it gets the initialization command from the MCU.  
NCP always starts with the initial baud rate, so Main MCU needs to try both initial and configured baud rate (to cover the case of MCU crash/reboot when NCP keeps running).

In some cases, Main MCU needs to restart the NCP initialization process:

- NCP upgrades it's own firmware Over the Air and gracefully reboots
- NCP reboots due to a crash (without any notification to the MCU)
- NCP locks up

There are several ways MCU can detect these cases.

### Graceful reboot indication

Before NCP is rebooting, it sends `RPC_EVENT_NCP_REBOOTING` event to the Main MCU via `rpc_client_processEvent()`.

### Periodic heartbeat

**Main MCU** should invoke a command to NCP every 15 seconds.  
If it **timeouts** 3 times in a row, MCU should try to restore the communication, by taking these steps:

1. If NCP was configured to baud rate other than the default, Main MCU can try reverting to the default one and restart the initialization sequence
2. If it fails, **Main MCU** should physically reset the ESP32 module (if possible on the hardware level), then start the initialization sequence

If there are no commands from Main MCU for 25 seconds, NPC will send a ping command to MCU.
- It will retry 5 times with the interval of 5 seconds
- If no reply, NCP will reboot/return into the `NOT_INITIALIZED` state
- The device will be `Offline` in the Blynk Cloud/App

### State check

If MCU always communicates with NCP on the initial/default baud rate, this may be useful:
When `rpc_blynk_getState()` returns `BLYNK_STATE_NOT_INITIALIZED`, MCU should initialize the NCP.  
It can also be combined with the periodic heartbeat.

