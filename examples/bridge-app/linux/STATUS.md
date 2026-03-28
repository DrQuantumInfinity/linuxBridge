# Matter Bridge Port — Status

## What was done

Ported the custom Matter bridge from a ~2020 fork (DrQuantumInfinity/linuxBridge) to current upstream connectedhomeip SDK (March 2026). The code lives on branch `upstream-port` at `github.com:DrQuantumInfinity/linuxBridge.git`.

### API migrations completed
- `EmberAfStatus` → `chip::Protocols::InteractionModel::Status`
- `EmberAfDeviceType` fields: `.deviceId` → `.deviceTypeId`, `.deviceVersion` → `.deviceTypeRevision`
- `ArraySize` → `MATTER_ARRAY_SIZE` (compat macro in Cluster.h)
- `registerAttributeAccessOverride` → removed (new delegate pattern in bridged-actions-stub.cpp)
- `emberAfActionsClusterInstantActionCallback` → `Actions::Delegate` class
- `MATTER_ATTRIBUTE_FLAG_READABLE` added to all external storage attributes (required by new SDK)
- `DeviceCallbacks.h/cpp` — stripped ESP32-specific code, added color control server stubs
- Various `-Wconversion` fixes for clang cross-compilation
- `BUILD.gn` — conditional pigpio/lib_dirs for x86_64 vs aarch64, added bridged-device-basic-information-server dep

### Build setup
- **Local x86_64**: `source scripts/activate.sh && cd examples/bridge-app/linux && gn gen out/debug && ninja -C out/debug`
- **Cross-compile aarch64**: `export SYSROOT_AARCH64=/opt/raspbian/sysroot && ./scripts/build/build_examples.py --target linux-arm64-bridge-clang build`
- Pi sysroot at `/opt/raspbian/sysroot` (rsynced from oldlaptop:/opt/raspbian/sysroot)

### Test suite
- `test/run_tests.py` — 11 tests, all passing
- Commissions via `already-discovered 127.0.0.1 5540` (no mDNS needed)
- All chip-tool calls use `--timeout 5` to avoid mDNS fallback hangs
- Run: `python3 examples/bridge-app/linux/test/run_tests.py`

### Deployment
- Bridge Pi: `192.168.0.128`, user `ubuntu`, ssh alias `bridgePi`
- Service: `/etc/systemd/system/bridge.service` (runs as ubuntu from /home/ubuntu)
- Deploy: `ssh bridgePi "sudo systemctl stop bridge" && scp out/linux-arm64-bridge-clang/chip-bridge-app bridgePi:~ && ssh bridgePi "sudo systemctl start bridge"`
- Config files: `mqttDeviceNames.conf` and `pingDevices.conf` need to be in working dir (`/home/ubuntu/`)
- KVS/INI files still go to `/tmp` (lost on reboot). BUILD.gn has defines for `FATCONFDIR`/`SYSCONFDIR`/`LOCALSTATEDIR` pointing to `/home/ubuntu/matter-data` but those only affect the bridge binary, not the SDK platform code's separate ini paths.

## What still needs work

### Critical
- **Persistence across reboots**: KVS goes to `--KVS` path but `chip_factory.ini`, `chip_config.ini`, `chip_counters.ini` still go to `/tmp`. Need to either patch `CHIPLinuxStorage.h` defaults or pass defines through the SDK's platform build (not just the bridge's BUILD.gn).
- **Re-commission after deploy**: The Pi bridge was freshly commissioned but previous Google Home/Apple Home fabric was lost when we wiped KVS. Needs re-pairing with the home controller.

### Improvements for maintainability (moving toward lamp-style decoupling)
1. Move to `AttributeAccessInterface` per cluster instead of global external callback dispatch
2. Use SDK's built-in cluster server implementations (OnOff, LevelControl, ColorControl) instead of reimplementing Read/Write
3. Replace manual `EmberAfCluster` struct construction with `DECLARE_DYNAMIC_CLUSTER` macros
4. Move custom code to its own repo that references the SDK as a submodule
5. Register dynamic endpoints with `CodegenDataModelProvider::Instance().Registry().Register()` (like upstream bridge example does)

### Minor
- `ModeCluster.cpp` and `DescriptorCluster.cpp` are entirely commented out — dead code, can delete
- `tasks/MatterTask.cpp` and `tasks/SerialTask.cpp` use FreeRTOS (ESP32-only) — not compiled for Linux, could be removed
- `utils/DynamicList.cpp` also FreeRTOS-only — same
- Null check needed in `TransportMqtt::Init()` / `TransportPing::Init()` when MQTT broker is unavailable (segfaults currently)
- `TempCluster.cpp` has operator precedence bug: `_temp = (int16_t) temp * 100` should be `(int16_t)(temp * 100)`

## Pairing info
- **QR Code:** https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3A-24J042C00KA0648G00
- **Manual code:** 34970112332
- **Passcode:** 20202021
- **Discriminator:** 3840
