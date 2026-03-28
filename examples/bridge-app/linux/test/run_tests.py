#!/usr/bin/env python3
"""Regression tests for the Matter Linux Bridge.

Starts the bridge, commissions it with chip-tool, then tests static endpoints,
dynamic MQTT device creation, and attribute read/write.

Prerequisites: mosquitto running, bridge + chip-tool built.
"""

import subprocess
import os
import sys
import time
import signal
import shutil
import tempfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BRIDGE_DIR = os.path.dirname(SCRIPT_DIR)
MATTER_ROOT = os.path.abspath(os.path.join(BRIDGE_DIR, "..", "..", ".."))

BRIDGE_BIN = os.path.join(BRIDGE_DIR, "out", "debug", "chip-bridge-app")
CHIP_TOOL = os.path.join(MATTER_ROOT, "out", "linux-x64-chip-tool", "chip-tool")

PASSCODE = "20202021"
NODE_ID = "1"
RESULT_FILE = "/tmp/bridge-test-results.txt"

GREEN = "\033[0;32m"
RED = "\033[0;31m"
YELLOW = "\033[0;33m"
NC = "\033[0m"

passed = 0
failed = 0
_result_fh = open(RESULT_FILE, "w")


def _out(msg):
    print(msg, flush=True)
    _result_fh.write(msg + "\n")
    _result_fh.flush()


def log(tag, msg):
    _out(f"{YELLOW}[{tag}]{NC} {msg}")


def pass_test(msg):
    global passed
    passed += 1
    _out(f"{GREEN}[PASS]{NC} {msg}")


def fail_test(msg, detail=""):
    global failed
    failed += 1
    _out(f"{RED}[FAIL]{NC} {msg}: {detail}")


def run(args, timeout=15):
    """Run a command, return (stdout+stderr, returncode)."""
    try:
        r = subprocess.run(args, capture_output=True, text=True, timeout=timeout)
        return r.stdout + r.stderr, r.returncode
    except subprocess.TimeoutExpired:
        return "", -1


def chip_tool(*args, timeout=15):
    """Run chip-tool with storage dir and return output."""
    cmd = [CHIP_TOOL] + list(args) + ["--storage-directory", tool_storage]
    output, _ = run(cmd, timeout=timeout)
    return output


def mqtt_pub(topic, payload):
    subprocess.run(["mosquitto_pub", "-t", topic, "-m", payload],
                   capture_output=True, timeout=5)


# ── Preflight ─────────────────────────────────────────────────────────────────

log("SETUP", "Checking prerequisites...")
for path, name in [(BRIDGE_BIN, "bridge"), (CHIP_TOOL, "chip-tool")]:
    if not os.path.isfile(path):
        print(f"Missing {name}: {path}")
        sys.exit(1)

r = subprocess.run(["pgrep", "-x", "mosquitto"], capture_output=True)
if r.returncode != 0:
    print("mosquitto not running. Start with: mosquitto -d")
    sys.exit(1)

# Kill any leftover bridge
subprocess.run(["pkill", "-f", "chip-bridge-app"], capture_output=True)
time.sleep(1)

# ── Setup ─────────────────────────────────────────────────────────────────────

test_dir = tempfile.mkdtemp(prefix="bridge-test-")
tool_storage = os.path.join(test_dir, "ts")
os.makedirs(tool_storage)

# Copy config files
for conf in ["mqttDeviceNames.conf", "pingDevices.conf"]:
    src = os.path.join(BRIDGE_DIR, conf)
    if os.path.exists(src):
        shutil.copy(src, test_dir)

bridge_log_path = os.path.join(test_dir, "bridge.log")
bridge_proc = None

try:
    # ── Start bridge ──────────────────────────────────────────────────────

    log("SETUP", "Starting bridge...")
    bridge_log = open(bridge_log_path, "w")
    bridge_proc = subprocess.Popen(
        [BRIDGE_BIN, "--KVS", os.path.join(test_dir, "kvs")],
        stdout=bridge_log, stderr=bridge_log,
        cwd=test_dir
    )
    time.sleep(4)

    if bridge_proc.poll() is not None:
        fail_test("Bridge startup", "process died")
        sys.exit(1)

    with open(bridge_log_path) as f:
        blog = f.read()
    if "Server Listening" not in blog:
        fail_test("Bridge startup", "not listening")
        sys.exit(1)
    pass_test("Bridge started and listening")

    # ── Commission ────────────────────────────────────────────────────────

    log("TEST", "Commissioning...")
    output = chip_tool("pairing", "onnetwork", NODE_ID, PASSCODE, timeout=90)
    time.sleep(1)

    with open(bridge_log_path) as f:
        blog = f.read()
    if "Commissioning completed successfully" in blog:
        pass_test("Commissioned")
    else:
        fail_test("Commission", "not in bridge log")
        print(output[-500:])
        sys.exit(1)

    if bridge_proc.poll() is not None:
        fail_test("Bridge died after commission", "")
        sys.exit(1)
    pass_test("Bridge alive after commission")

    # ── Read vendor ID (EP0) ──────────────────────────────────────────────

    log("TEST", "Read vendor ID (EP0)...")
    output = chip_tool("basicinformation", "read", "vendor-id", NODE_ID, "0")
    if "0xFFF1" in output or "65521" in output:
        pass_test("Vendor ID (0xFFF1)")
    else:
        fail_test("Vendor ID", output[:200])

    # ── Read bridge descriptor (EP1) ──────────────────────────────────────

    log("TEST", "Read descriptor (EP1)...")
    output = chip_tool("descriptor", "read", "device-type-list", NODE_ID, "1")
    if "DeviceTypeList" in output or "deviceType" in output.lower():
        pass_test("Bridge descriptor (EP1)")
    else:
        fail_test("Bridge descriptor", output[:200])

    # ── MQTT device creation ──────────────────────────────────────────────

    log("TEST", "Creating MQTT dimmer...")
    mqtt_pub("DimmerFeit/AABBCCDDEEFF/1/get", "1")
    time.sleep(3)

    with open(bridge_log_path) as f:
        blog = f.read()

    device_ep = None
    for line in blog.splitlines():
        if "Adding device" in line:
            # "Adding device N: NAME"
            parts = line.split("Adding device ")
            if len(parts) > 1:
                idx_str = parts[1].split(":")[0].strip()
                try:
                    device_ep = 3 + int(idx_str)
                except ValueError:
                    pass
            break

    if device_ep and "Added device" in blog:
        pass_test(f"MQTT device created (EP{device_ep})")
    else:
        fail_test("MQTT device creation", "not in bridge log")
        device_ep = 4  # fallback

    ep = str(device_ep)

    # ── Read OnOff ────────────────────────────────────────────────────────

    log("TEST", f"Read OnOff (EP{ep})...")
    output = chip_tool("onoff", "read", "on-off", NODE_ID, ep)
    if "OnOff" in output or "on-off" in output.lower():
        pass_test("Read OnOff")
    else:
        fail_test("Read OnOff", output[:300])

    # ── Read NodeLabel ────────────────────────────────────────────────────

    log("TEST", f"Read NodeLabel (EP{ep})...")
    output = chip_tool("bridgeddevicebasicinformation", "read", "node-label", NODE_ID, ep)
    if "NodeLabel" in output or "DimmerFeit" in output or "node-label" in output.lower():
        pass_test("Read NodeLabel")
    else:
        fail_test("Read NodeLabel", output[:300])

    # ── MQTT OFF → Matter ─────────────────────────────────────────────────

    log("TEST", "MQTT OFF → Matter...")
    mqtt_pub("DimmerFeit/AABBCCDDEEFF/1/get", "0")
    time.sleep(2)
    output = chip_tool("onoff", "read", "on-off", NODE_ID, ep)
    if "FALSE" in output or "false" in output.lower() or "value: 0" in output:
        pass_test("MQTT OFF reflected in Matter")
    else:
        fail_test("MQTT OFF reflected", output[:300])

    # ── Matter ON command ─────────────────────────────────────────────────

    log("TEST", "Matter ON command...")
    output = chip_tool("onoff", "on", NODE_ID, ep)
    if "Received Command Response Status" in output or "status" in output.lower():
        pass_test("Matter OnOff ON command")
    else:
        fail_test("Matter OnOff ON", output[:300])

    # ── Read LevelControl ─────────────────────────────────────────────────

    log("TEST", f"Read LevelControl (EP{ep})...")
    output = chip_tool("levelcontrol", "read", "current-level", NODE_ID, ep)
    if "CurrentLevel" in output or "currentLevel" in output or "current-level" in output.lower():
        pass_test("Read LevelControl")
    else:
        fail_test("Read LevelControl", output[:300])

    # ── Second MQTT device ────────────────────────────────────────────────

    log("TEST", "Second MQTT device...")
    mqtt_pub("DimmerFeit/112233445566/1/get", "1")
    time.sleep(3)

    with open(bridge_log_path) as f:
        blog = f.read()
    count = blog.count("Added device")
    if count >= 2:
        pass_test(f"Second device created ({count} total)")
    else:
        fail_test("Second device", f"only {count} in log")

finally:
    # ── Cleanup ───────────────────────────────────────────────────────────
    if bridge_proc and bridge_proc.poll() is None:
        bridge_proc.terminate()
        try:
            bridge_proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            bridge_proc.kill()

    if failed == 0:
        shutil.rmtree(test_dir, ignore_errors=True)
    else:
        print(f"{YELLOW}Test artifacts kept at: {test_dir}{NC}")

# ── Summary ───────────────────────────────────────────────────────────────────

_out("")
_out("════════════════════════════════════════")
_out(f"  Results: {GREEN}{passed} passed{NC}, {RED}{failed} failed{NC} (of {passed + failed})")
_out("════════════════════════════════════════")
_result_fh.close()
sys.exit(failed)
