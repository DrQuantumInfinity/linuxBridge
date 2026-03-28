# Matter Linux Bridge

A Matter bridge that exposes MQTT devices (Feit dimmers, outlets, RGB lamps) and ESP-NOW devices to a Matter fabric. Runs on Raspberry Pi.

## Pairing

- **QR Code:** https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3A-24J042C00KA0648G00
- **Manual pairing code:** 34970112332
- **Passcode:** 20202021
- **Discriminator:** 3840

## Deployment

The bridge runs on the Pi at `192.168.0.128` (user `ubuntu`).

```sh
# Cross-compile from pop-os
cd ~/projects/matter
source scripts/activate.sh
export SYSROOT_AARCH64=/opt/raspbian/sysroot
./scripts/build/build_examples.py --target linux-arm64-bridge-clang build

# Deploy
scp out/linux-arm64-bridge-clang/chip-bridge-app bridgePi:~
ssh bridgePi "sudo systemctl restart bridge"
```

The systemd service is at `/etc/systemd/system/bridge.service`.

## Building locally (x86_64, for validation)

```sh
cd ~/projects/matter
source scripts/activate.sh
cd examples/bridge-app/linux
gn gen out/debug
ninja -C out/debug
```

## Transport layers

- **MQTT** — Subscribes to `DimmerFeit/#`, `OutletGordon/#`, `RgbLampGordon/#` on the local broker. Device names can be mapped in `mqttDeviceNames.conf`.
- **ESP-NOW** — Serial protocol over `/dev/ttyEspNow` for ESP-NOW devices (DHT sensors, RGB lights, toggles).
- **Ping** — ICMP ping-based presence detection. Add/remove via MQTT topics `ping/addIP` and `ping/remIP`. Static devices in `pingDevices.conf`.

## Persistence

- `mqttPersist.bin` / `espnowPersist.bin` — device lists (in working directory)
- `/tmp/chip_kvs` — Matter KVS (commissioning state, fabrics, ACLs)
- `/tmp/chip_factory.ini`, `chip_config.ini`, `chip_counters.ini` — SDK config

Note: `/tmp` files are lost on reboot. Use `--KVS ~/matter-data/chip_kvs` to persist the KVS elsewhere. The INI paths are hardcoded in the SDK and would need compile-time overrides (`FATCONFDIR`, `SYSCONFDIR`, `LOCALSTATEDIR` defines) to relocate.
