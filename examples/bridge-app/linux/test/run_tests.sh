#!/bin/bash
# Regression tests for the Matter Linux Bridge
#
# Prerequisites: mosquitto running, bridge + chip-tool built
# Usage: ./test/run_tests.sh

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BRIDGE_DIR="$(dirname "$SCRIPT_DIR")"
MATTER_ROOT="$(cd "$BRIDGE_DIR/../../.." && pwd)"

BRIDGE_BIN="$BRIDGE_DIR/out/debug/chip-bridge-app"
CHIP_TOOL="$MATTER_ROOT/out/linux-x64-chip-tool/chip-tool"
TEST_DIR="/tmp/bridge-test-$$"
BRIDGE_PID=""
PASS=0
FAIL=0
PASSCODE=20202021
NODE_ID=1

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

cleanup() {
    [ -n "$BRIDGE_PID" ] && kill "$BRIDGE_PID" 2>/dev/null && wait "$BRIDGE_PID" 2>/dev/null
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

log()  { echo -e "${YELLOW}[$1]${NC} $2"; }
pass() { PASS=$((PASS + 1)); echo -e "${GREEN}[PASS]${NC} $1"; }
fail() { FAIL=$((FAIL + 1)); echo -e "${RED}[FAIL]${NC} $1: $2"; }

ct() {
    timeout 15 "$CHIP_TOOL" "$@" --storage-directory "$TEST_DIR/ts" 2>&1
}

# ── Preflight ─────────────────────────────────────────────────────────────────

log "SETUP" "Checking prerequisites..."
[ -x "$BRIDGE_BIN" ] || { echo "Missing bridge binary"; exit 1; }
[ -x "$CHIP_TOOL" ] || { echo "Missing chip-tool"; exit 1; }
pgrep -x mosquitto > /dev/null || { echo "mosquitto not running"; exit 1; }
pkill -f "chip-bridge-app" 2>/dev/null; sleep 1

# ── Start bridge ──────────────────────────────────────────────────────────────

mkdir -p "$TEST_DIR/ts"
cp "$BRIDGE_DIR/mqttDeviceNames.conf" "$TEST_DIR/" 2>/dev/null || true
cd "$TEST_DIR"

log "SETUP" "Starting bridge..."
"$BRIDGE_BIN" --KVS "$TEST_DIR/kvs" > "$TEST_DIR/bridge.log" 2>&1 &
BRIDGE_PID=$!
sleep 4
kill -0 "$BRIDGE_PID" 2>/dev/null || { fail "Bridge start" "died"; exit 1; }
grep -q "Server Listening" "$TEST_DIR/bridge.log" || { fail "Bridge start" "not listening"; exit 1; }
pass "Bridge started"

# ── Commission ────────────────────────────────────────────────────────────────

log "TEST" "Commissioning..."
timeout 90 "$CHIP_TOOL" pairing onnetwork "$NODE_ID" "$PASSCODE" \
    --storage-directory "$TEST_DIR/ts" > "$TEST_DIR/commission.log" 2>&1 || true
sleep 1
grep -q "Commissioning completed successfully" "$TEST_DIR/bridge.log" || { fail "Commission" "failed"; tail -5 "$TEST_DIR/commission.log"; exit 1; }
kill -0 "$BRIDGE_PID" 2>/dev/null || { fail "Bridge died" ""; exit 1; }
pass "Commissioned"

# ── Static endpoint tests ─────────────────────────────────────────────────────

log "TEST" "Read vendor ID (EP0)..."
ct basicinformation read vendor-id "$NODE_ID" 0 | grep -q "0xFFF1\|65521" && pass "Vendor ID" || fail "Vendor ID" ""

log "TEST" "Read descriptor (EP1)..."
ct descriptor read device-type-list "$NODE_ID" 1 | grep -qi "devicetype" && pass "Bridge descriptor" || fail "Bridge descriptor" ""

# ── MQTT dynamic device ───────────────────────────────────────────────────────

log "TEST" "Creating MQTT dimmer..."
mosquitto_pub -t "DimmerFeit/AABBCCDDEEFF/1/get" -m "1"
sleep 3

DEVICE_INDEX=$(grep "Adding device" "$TEST_DIR/bridge.log" | head -1 | grep -oP 'Adding device \K[0-9]+')
DEVICE_EP=$((3 + ${DEVICE_INDEX:-1}))
grep -q "Added device" "$TEST_DIR/bridge.log" && pass "MQTT device created (EP$DEVICE_EP)" || fail "MQTT device" "not in log"

log "TEST" "Read OnOff (EP$DEVICE_EP)..."
ct onoff read on-off "$NODE_ID" "$DEVICE_EP" | grep -qi "onoff" && pass "Read OnOff" || fail "Read OnOff" ""

log "TEST" "Read NodeLabel (EP$DEVICE_EP)..."
ct bridgeddevicebasicinformation read node-label "$NODE_ID" "$DEVICE_EP" | grep -qi "nodelabel\|dimmer" && pass "NodeLabel" || fail "NodeLabel" ""

log "TEST" "MQTT OFF → Matter..."
mosquitto_pub -t "DimmerFeit/AABBCCDDEEFF/1/get" -m "0"
sleep 2
ct onoff read on-off "$NODE_ID" "$DEVICE_EP" | grep -qi "false\|value: 0" && pass "MQTT OFF reflected" || fail "MQTT OFF" ""

log "TEST" "Matter ON command..."
ct onoff on "$NODE_ID" "$DEVICE_EP" | grep -qi "status" && pass "OnOff ON cmd" || fail "OnOff ON" ""

log "TEST" "Read LevelControl..."
ct levelcontrol read current-level "$NODE_ID" "$DEVICE_EP" | grep -qi "currentlevel" && pass "LevelControl" || fail "LevelControl" ""

log "TEST" "Second MQTT device..."
mosquitto_pub -t "DimmerFeit/112233445566/1/get" -m "1"
sleep 3
SECOND=$(grep -c "Added device" "$TEST_DIR/bridge.log")
[ "$SECOND" -ge 2 ] && pass "Second device created" || fail "Second device" "count=$SECOND"

# ── Summary ───────────────────────────────────────────────────────────────────

echo ""
echo "════════════════════════════════════════"
echo -e "  Results: ${GREEN}$PASS passed${NC}, ${RED}$FAIL failed${NC} (of $((PASS+FAIL)))"
echo "════════════════════════════════════════"
exit "$FAIL"
