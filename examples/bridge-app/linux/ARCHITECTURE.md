# Bridge App Architecture

A Matter protocol bridge built on the CHIP/connectedhomeip SDK, running on Linux (Raspberry Pi). It dynamically bridges external IoT devices into the Matter ecosystem so they appear in Google Home, HomeKit, etc.

## Layered Architecture

```
┌─────────────────────────────────────────────────┐
│            Matter Framework (CHIP SDK)           │
│     (Endpoint management, subscriptions)         │
├─────────────────────────────────────────────────┤
│              Endpoint API Layer                  │
│  (Dynamic endpoint create/remove, attribute      │
│   read/write callbacks, change reporting)         │
│  utils/EndpointApi.cpp                           │
├─────────────────────────────────────────────────┤
│               Device Layer                       │
│  (Base Device + specialized types, each owning   │
│   a vector of Clusters + a TransportLayer*)      │
│  devices/Device*.cpp                             │
├─────────────────────────────────────────────────┤
│              Cluster Layer                       │
│  (1:1 mapping to Matter clusters, all using      │
│   EXTERNAL_STORAGE — state in cluster objects)   │
│  clusters/*Cluster.cpp                           │
├─────────────────────────────────────────────────┤
│           Transport Layer (plugins)              │
│  ┌─────────────┬──────────┬──────────┐           │
│  │  ESP-NOW    │  MQTT    │  Ping    │           │
│  │  (serial)   │ (broker) │ (HTTP)   │           │
│  └─────────────┴──────────┴──────────┘           │
│  transportLayer/{espNow,mqtt,ping}/              │
├─────────────────────────────────────────────────┤
│         Serial / Task Layer                      │
│  (UART framing w/ CRC, FreeRTOS tasks)           │
│  tasks/SerialTask.cpp, tasks/MatterTask.cpp      │
└─────────────────────────────────────────────────┘
```

## Key Components

### Transport Layer (`transportLayer/`)

Base class: `TransportLayer` in `transportLayer.h` with virtual `Send()`.

- **ESP-NOW** (`transportLayer/espNow/`) — ESP32 devices over serial UART. Binary packets with device type, MAC address, and type-specific payload. Frame delimiter `0x1f5a3db9` + CRC16.
  - Device types: DHT (temp/humidity), Toggle (button), Light RGB, Light Dimmer, Debug
- **MQTT** (`transportLayer/mqtt/`) — Smart home devices via MQTT broker. Topic format: `{DeviceType}/{MAC}/{command_id}/set`.
  - Device types: Feit dimmers, Gordon outlets, RGB lamps
- **Ping** (`transportLayer/ping/`) — Simple HTTP/network-based devices, IP-addressed.

Each transport auto-creates devices on first message and persists known devices to binary files via `PersistDevList`.

### Device Layer (`devices/`)

Base class `Device` owns:
- `std::vector<Cluster*> _clusters` — composed clusters
- `TransportLayer* _pTransportLayer` — communication back-channel
- `BasicCluster basicCluster` — always present (BridgedDeviceBasicInformation)
- `ENDPOINT_DATA` struct for Matter registration

Specializations (each composes the clusters it needs):

| Type | Clusters | Purpose |
|------|----------|---------|
| `DeviceLight` | OnOff, Descriptor, Basic | Simple on/off light |
| `DeviceLightLevel` | OnOff, LevelControl, Descriptor, Basic | Dimmable light |
| `DeviceLightRGB` | OnOff, LevelControl, Colour, Descriptor, Basic | RGB color light |
| `DeviceLightTemp` | OnOff, Colour (temp mode), Descriptor, Basic | Color temperature light |
| `DeviceButton` | OnOff, Descriptor, Basic | Button/switch |
| `DeviceTemperature` | Temp, Humidity, Descriptor, Basic | Temperature/humidity sensor |
| `DevicePing` | — | Network-based ping device |

### Cluster Layer (`clusters/`)

Base class `Cluster` in `Cluster.h` with virtual `Read()` / `Write()`.

| Cluster | Matter Cluster | Key Attributes |
|---------|---------------|----------------|
| `OnOffCluster` | OnOff (0x0006) | OnOff (bool) |
| `LevelControlCluster` | LevelControl (0x0008) | CurrentLevel (0-255), MinLevel, MaxLevel |
| `ColourCluster` | ColorControl (0x0300) | Hue, Saturation, ColorTemp (dual mode: HSV or temp) |
| `TempCluster` | TemperatureMeasurement (0x0402) | MeasuredValue (int16, 1/100 C) |
| `HumidityCluster` | RelativeHumidityMeasurement (0x0405) | MeasuredValue (0-10000) |
| `BasicCluster` | BridgedDeviceBasicInformation (0x0039) | NodeLabel, Reachable |
| `DescriptorCluster` | Descriptor (0x001D) | Device type list |
| `ModeCluster` | ModeBase | Mode selection |

All clusters use `ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)` — attribute state lives in cluster objects, not ZAP-generated buffers. `SetXxx()` methods update state and call `EndpointReportChange()` to notify Matter subscribers.

### Endpoint API (`utils/EndpointApi.cpp`)

Manages dynamic Matter endpoints at runtime:

- `EndpointApiInit()` — Sets up EP0 (root) and EP1 (bridge aggregate), disables ZAP placeholder, calculates first dynamic endpoint ID
- `EndpointAdd(ENDPOINT_DATA*)` — Scheduled via `PlatformMgr().ScheduleWork()`, calls `emberAfSetDynamicEndpoint()`
- `EndpointRemove(deviceIndex)` — Clears dynamic endpoint slot
- `EndpointReportChange(deviceIndex, clusterId, attrId)` — Converts to `ConcreteAttributePath`, triggers `MatterReportingAttributeChangeCallback()`
- `emberAfExternalAttributeReadCallback()` / `WriteCallback()` — Routes Matter attribute ops to Device objects

### Utils (`utils/`)

- `DeviceList` — Temporary in-memory device cache with expiry (1-day TTL)
- `PersistDevList` — Binary file persistence for known devices (survives restarts)
- `DynamicList` — Generic dynamic array utility
- `Crc16` — CRC16 for serial frame validation
- `uart` — UART driver abstraction
- `pingUtil` — Network ping utilities
- `timer` — Timer and sleep utilities
- `futil` — File utilities
- `Log` — Logging with configurable levels

### Tasks (`tasks/`)

- `SerialTask` — UART RX/TX in its own thread. Accumulates bytes, detects frame delimiters, validates CRC, dispatches to `TransportEspNow::HandleSerialRx()`
- `MatterTask` — Matter framework task wrapper

### Device Callbacks (`DeviceCallbacks.cpp`)

- `PostAttributeChangeCallback()` — Logs attribute changes
- `ActionsAttrAccess` — Attribute access override for Actions cluster (returns empty lists)

## Data Flow

### Inbound (external device -> Matter controllers)

```
External Device
  -> Transport RX handler (HandleSerialRx / HandleTopicRx)
    -> Lookup/create Device in DeviceList
      -> Update Cluster state (SetOn, SetLevel, etc.)
        -> EndpointReportChange()
          -> MatterReportingAttributeChangeCallback()
            -> Matter notifies subscribed controllers (Google Home, etc.)
```

### Outbound (Matter controller -> external device)

```
Controller writes attribute
  -> emberAfExternalAttributeWriteCallback()
    -> Device::WriteCluster()
      -> Cluster::Write() updates internal state
        -> TransportLayer::Send()
          -> Construct protocol-specific packet
            -> Transmit to external device
```

## Persistence

Each transport maintains its own `PersistDevList` file:
- ESP-NOW: `espnowPersist.bin`
- MQTT: `mqttPersist.bin`

On startup, persisted devices are restored so the bridge remembers its devices across restarts.

## Thread Safety

- Endpoint operations scheduled through `PlatformMgr().ScheduleWork()` (Matter's event loop)
- Serial I/O runs in its own FreeRTOS task
- Transport RX handlers dispatch into Matter's thread via scheduled work

## Building & Deployment

See `README.md` for build instructions. Deployed as a systemd service (`bridge.service`) on Raspberry Pi. Cross-compilation scripts available (`crossCompile.sh`).

## Key File Locations

| Area | Path |
|------|------|
| Entry point | `main.cpp` |
| Device types | `devices/Device*.{h,cpp}` |
| Cluster implementations | `clusters/*Cluster.{h,cpp}` |
| Endpoint management | `utils/EndpointApi.{h,cpp}` |
| Transport base | `transportLayer/transportLayer.h` |
| ESP-NOW transport | `transportLayer/espNow/` |
| MQTT transport | `transportLayer/mqtt/` |
| Ping transport | `transportLayer/ping/` |
| Device storage | `utils/DeviceList.{h,cpp}`, `utils/PersistDevList.{h,cpp}` |
| Serial framing | `tasks/SerialTask.{h,cpp}` |
| Callbacks | `DeviceCallbacks.{h,cpp}` |
| Build config | `BUILD.gn`, `args.gn` |
