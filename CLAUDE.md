# CLAUDE.md

This is a fork of the connectedhomeip (Matter) SDK. Most of the SDK is upstream code — the custom work lives in:

## Focus Area

**`examples/bridge-app/linux/`** — A Matter bridge for Linux (Raspberry Pi) that dynamically bridges ESP-NOW, MQTT, and HTTP devices into the Matter ecosystem.

See `examples/bridge-app/linux/ARCHITECTURE.md` for a detailed architecture guide covering the layered design, device/cluster/transport abstractions, data flow, and file locations.

## Building

```sh
cd examples/bridge-app/linux
source third_party/connectedhomeip/scripts/activate.sh
gn gen out/debug
ninja -C out/debug
```

Cross-compilation scripts: `crossCompile.sh`, `crossCompilePaul.sh`

## Key Conventions

- Devices are composed from clusters (not deep inheritance)
- All cluster attributes use `EXTERNAL_STORAGE` — state lives in cluster objects
- Endpoint operations must be scheduled via `PlatformMgr().ScheduleWork()`
- Transport layers auto-create devices on first message and persist them to binary files
- The bridge runs as a systemd service on Raspberry Pi
