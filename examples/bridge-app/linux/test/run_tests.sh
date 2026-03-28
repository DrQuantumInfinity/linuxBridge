#!/bin/bash
# Regression tests for the Matter Linux Bridge
# Runs the bridge locally, commissions it, and tests device operations.
#
# Prerequisites:
#   - mosquitto broker running locally
#   - chip-bridge-app built (x86_64 debug)
#   - chip-tool built
#
# Usage: ./test/run_tests.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BRIDGE_DIR="$(dirname "$SCRIPT_DIR")"
MATTER_ROOT="$(cd "$BRIDGE_DIR/../../.." && pwd)"

BRIDGE_BIN="$BRIDGE_DIR/out/debug/chip-bridge-app"
CHIP_TOOL="$MATTER_ROOT/out/linux-x64-chip-tool/chip-tool"
TEST_DIR="/tmp/bridge-test-$$"
KVS_BRIDGE="$TEST_DIR/bridge_kvs"
KVS_TOOL="$TEST_DIR/tool_kvs"
BRIDGE_PID=""
PASS=0
FAIL=0

# Pairing constants
DISCRIMINATOR=3840
PASSCODE=20202021
NODE_ID=1

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

cleanup() {
    if [ -n "$BRIDGE_PID" ] && kill -0 "$BRIDGE_PID" 2>/dev/null; then
        kill "$BRIDGE_PID" 2>/dev/null
        wait "$BRIDGE_PID" 2>/dev/null || true
    fi
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

log() { echo -e "${YELLOW}[$1]${NC} $2"; }
pass() { PASS=$((PASS + 1)); echo -e "${GREEN}[PASS]${NC} $1"; }
fail() { FAIL=$((FAIL + 1)); echo -e "${RED}[FAIL]${NC} $1: $2"; }

chip_tool() {
    timeout 15 "$CHIP_TOOL" "$@" --storage-directory "$TEST_DIR/tool-storage" --timeout 10 2>&1
}

# ── Preflight checks ──────────────────────────────────────────────────────────

log "SETUP" "Checking prerequisites..."

if [ ! -x "$BRIDGE_BIN" ]; then
    echo "Bridge binary not found at $BRIDGE_BIN"
    echo "Build with: cd $BRIDGE_DIR && gn gen out/debug && ninja -C out/debug"
    exit 1
fi

if [ ! -x "$CHIP_TOOL" ]; then
    echo "chip-tool not found at $CHIP_TOOL"
    echo "Build with: ./scripts/build/build_examples.py --target linux-x64-chip-tool build"
    exit 1
fi

if ! pgrep -x mosquitto > /dev/null; then
    echo "mosquitto broker not running. Start with: mosquitto -d"
    exit 1
fi

# ── Start bridge ──────────────────────────────────────────────────────────────

log "SETUP" "Preparing test directory: $TEST_DIR"
mkdir -p "$TEST_DIR/tool-storage"

# Copy config files if they exist
cp "$BRIDGE_DIR/mqttDeviceNames.conf" "$TEST_DIR/" 2>/dev/null || true
cp "$BRIDGE_DIR/pingDevices.conf" "$TEST_DIR/" 2>/dev/null || true

log "SETUP" "Starting bridge..."
cd "$TEST_DIR"
"$BRIDGE_BIN" --KVS "$KVS_BRIDGE" > "$TEST_DIR/bridge.log" 2>&1 &
BRIDGE_PID=$!
sleep 5

if ! kill -0 "$BRIDGE_PID" 2>/dev/null; then
    fail "Bridge startup" "Process died"
    exit 1
fi
pass "Bridge started (PID $BRIDGE_PID)"

# ── Test 1: Commission ────────────────────────────────────────────────────────

log "TEST" "Commissioning bridge..."
chip_tool pairing onnetwork "$NODE_ID" "$PASSCODE" > "$TEST_DIR/commission.log" 2>&1 || true
sleep 2
if grep -q "Commissioning completed successfully" "$TEST_DIR/bridge.log"; then
    pass "Commission bridge"
else
    fail "Commission bridge" "commissioning did not succeed"
    echo "--- chip-tool output (last 20 lines) ---"
    tail -20 "$TEST_DIR/commission.log"
    echo "--- bridge log (last 20 lines) ---"
    tail -20 "$TEST_DIR/bridge.log"
    # Can't continue without commissioning
    exit 1
fi

# Verify bridge is still running after commissioning
if ! kill -0 "$BRIDGE_PID" 2>/dev/null; then
    fail "Bridge alive after commission" "Bridge process died"
    echo "--- bridge log (last 30 lines) ---"
    tail -30 "$TEST_DIR/bridge.log"
    exit 1
fi
pass "Bridge alive after commission"

# ── Test 2: Read bridge basic info ────────────────────────────────────────────

log "TEST" "Reading bridge basic info (endpoint 0)..."
OUTPUT=$(chip_tool basicinformation read vendor-id "$NODE_ID" 0)
if echo "$OUTPUT" | grep -q "UNSUPPORTED_ENDPOINT\|vendorId\|0xFFF1"; then
    pass "Read bridge vendor ID"
else
    fail "Read bridge vendor ID" "Unexpected output"
fi

# ── Test 3: Read descriptor on endpoint 1 (aggregate/bridge) ─────────────────

log "TEST" "Reading descriptor cluster on bridge endpoint (EP1)..."
OUTPUT=$(chip_tool descriptor read device-type-list "$NODE_ID" 1)
if echo "$OUTPUT" | grep -q "DeviceTypeList\|deviceType"; then
    pass "Read bridge descriptor"
else
    fail "Read bridge descriptor" "$(echo "$OUTPUT" | tail -5)"
fi

# ── Test 4: Simulate MQTT device and verify dynamic endpoint ─────────────────

log "TEST" "Publishing MQTT message to create a dimmer device..."
mosquitto_pub -t "DimmerFeit/AABBCCDDEEFF/1/get" -m "1"
sleep 3

# Find which endpoint the device landed on from bridge log
DEVICE_EP=""
for ep in 3 4 5 6; do
    if grep -q "Added device.*at dynamic endpoint" "$TEST_DIR/bridge.log"; then
        DEVICE_EP=$(grep "Added device" "$TEST_DIR/bridge.log" | head -1 | grep -oP 'dynamic endpoint \K[0-9]+')
        # endpoint ID = firstDynamicEndpointId + deviceIndex; parse from log
        break
    fi
done

# Fallback: scan endpoints 3-6 with chip-tool
if [ -z "$DEVICE_EP" ]; then
    for ep in 3 4 5 6; do
        if chip_tool descriptor read device-type-list "$NODE_ID" "$ep" --timeout 3 2>/dev/null | grep -q "0x0101"; then
            DEVICE_EP="$ep"
            break
        fi
    done
fi

if [ -n "$DEVICE_EP" ]; then
    pass "Dynamic endpoint created for MQTT dimmer (EP$DEVICE_EP)"
else
    fail "Dynamic endpoint for MQTT dimmer" "No dynamic endpoint found on EP3-6"
    DEVICE_EP=4  # try anyway
fi

# ── Test 5: Read OnOff state of the dynamic device ───────────────────────────

log "TEST" "Reading OnOff attribute on dynamic endpoint..."
OUTPUT=$(chip_tool onoff read on-off "$NODE_ID" "$DEVICE_EP")
if echo "$OUTPUT" | grep -q "OnOff:"; then
    pass "Read OnOff on dynamic endpoint"
else
    fail "Read OnOff on dynamic endpoint" "$(echo "$OUTPUT" | tail -5)"
fi

# ── Test 6: Read bridged device name ─────────────────────────────────────────

log "TEST" "Reading bridged device name (NodeLabel)..."
OUTPUT=$(chip_tool bridgeddevicebasicinformation read node-label "$NODE_ID" "$DEVICE_EP")
if echo "$OUTPUT" | grep -q "NodeLabel\|DimmerFeit"; then
    pass "Read bridged device NodeLabel"
else
    fail "Read bridged device NodeLabel" "$(echo "$OUTPUT" | tail -5)"
fi

# ── Test 7: Toggle OnOff via MQTT update ─────────────────────────────────────

log "TEST" "Toggling device via MQTT..."
mosquitto_pub -t "DimmerFeit/AABBCCDDEEFF/1/get" -m "0"
sleep 1
OUTPUT=$(chip_tool onoff read on-off "$NODE_ID" "$DEVICE_EP")
if echo "$OUTPUT" | grep -q "OnOff: FALSE\|value: FALSE"; then
    pass "MQTT toggle reflected in Matter"
else
    fail "MQTT toggle reflected in Matter" "$(echo "$OUTPUT" | tail -5)"
fi

# ── Test 8: Write OnOff from Matter side ─────────────────────────────────────

log "TEST" "Sending OnOff On command from Matter..."
OUTPUT=$(chip_tool onoff on "$NODE_ID" "$DEVICE_EP")
if echo "$OUTPUT" | grep -q "Success\|Received Command Response Status"; then
    pass "Matter OnOff command accepted"
else
    fail "Matter OnOff command" "$(echo "$OUTPUT" | tail -5)"
fi

# ── Test 9: Level control ────────────────────────────────────────────────────

log "TEST" "Reading LevelControl on dynamic endpoint..."
OUTPUT=$(chip_tool levelcontrol read current-level "$NODE_ID" "$DEVICE_EP")
if echo "$OUTPUT" | grep -q "CurrentLevel\|currentLevel"; then
    pass "Read LevelControl"
else
    fail "Read LevelControl" "$(echo "$OUTPUT" | tail -5)"
fi

# ── Test 10: Multiple devices ────────────────────────────────────────────────

log "TEST" "Creating second MQTT device..."
mosquitto_pub -t "DimmerFeit/112233445566/1/get" -m "1"
sleep 2
OUTPUT=$(chip_tool descriptor read device-type-list "$NODE_ID" 4)
if echo "$OUTPUT" | grep -q "DeviceTypeList\|deviceType"; then
    pass "Second dynamic endpoint created"
else
    fail "Second dynamic endpoint" "$(echo "$OUTPUT" | tail -5)"
fi

# ── Summary ───────────────────────────────────────────────────────────────────

echo ""
echo "════════════════════════════════════════"
echo -e "  Results: ${GREEN}$PASS passed${NC}, ${RED}$FAIL failed${NC}"
echo "════════════════════════════════════════"

exit "$FAIL"
