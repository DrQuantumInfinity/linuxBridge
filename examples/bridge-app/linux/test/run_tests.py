#!/usr/bin/env python3
"""Regression tests for the Matter Linux Bridge.

Commissions via direct IP, reads via individual chip-tool calls.
"""

import subprocess, os, sys, time, re, shutil, tempfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BRIDGE_DIR = os.path.dirname(SCRIPT_DIR)
MATTER_ROOT = os.path.abspath(os.path.join(BRIDGE_DIR, "..", "..", ".."))
BRIDGE_BIN = os.path.join(BRIDGE_DIR, "out", "debug", "chip-bridge-app")
CHIP_TOOL = os.path.join(MATTER_ROOT, "out", "linux-x64-chip-tool", "chip-tool")
PASSCODE, NODE_ID = "20202021", "1"
RESULT_FILE = "/tmp/bridge-test-results.txt"

G = "\033[32m"; R = "\033[31m"; Y = "\033[33m"; N = "\033[0m"
passed = failed = 0
rf = open(RESULT_FILE, "w")

def out(m): print(m, flush=True); rf.write(m+"\n"); rf.flush()
def log(t, m): out(f"{Y}[{t}]{N} {m}")
def ok(m): global passed; passed += 1; out(f"{G}[PASS]{N} {m}")
def fail(m, d=""): global failed; failed += 1; out(f"{R}[FAIL]{N} {m}: {d[:150]}")
def strip(t): return re.sub(r'\x1b\[[0-9;]*m', '', t)

def ct(*args, timeout=15):
    """Run chip-tool, return ANSI-stripped output."""
    r = subprocess.run([CHIP_TOOL] + list(args) + ["--storage-directory", tool_storage],
                       capture_output=True, text=True, timeout=timeout)
    return strip(r.stdout + r.stderr)

def mqtt(topic, payload):
    subprocess.run(["mosquitto_pub", "-t", topic, "-m", payload], capture_output=True, timeout=5)

# ── Preflight ─────────────────────────────────────────────────────────────────
log("SETUP", "Checking prerequisites...")
assert os.path.isfile(BRIDGE_BIN), f"Missing {BRIDGE_BIN}"
assert os.path.isfile(CHIP_TOOL), f"Missing {CHIP_TOOL}"
assert subprocess.run(["pgrep", "-x", "mosquitto"], capture_output=True).returncode == 0, "mosquitto not running"
subprocess.run(["pkill", "-f", "chip-bridge-app"], capture_output=True); time.sleep(1)

# ── Setup ─────────────────────────────────────────────────────────────────────
test_dir = tempfile.mkdtemp(prefix="bridge-test-")
tool_storage = os.path.join(test_dir, "ts"); os.makedirs(tool_storage)
for c in ["mqttDeviceNames.conf", "pingDevices.conf"]:
    s = os.path.join(BRIDGE_DIR, c)
    if os.path.exists(s): shutil.copy(s, test_dir)

blog = os.path.join(test_dir, "bridge.log")
bp = None

try:
    log("SETUP", "Starting bridge...")
    bp = subprocess.Popen([BRIDGE_BIN, "--KVS", os.path.join(test_dir, "kvs")],
                          stdout=open(blog, "w"), stderr=subprocess.STDOUT, cwd=test_dir)
    time.sleep(4)
    assert bp.poll() is None, "bridge died"
    assert "Server Listening" in open(blog).read(), "not listening"
    ok("Bridge started")

    # ── Commission ────────────────────────────────────────────────────────
    log("TEST", "Commissioning via 127.0.0.1:5540...")
    ct("pairing", "already-discovered", NODE_ID, PASSCODE, "127.0.0.1", "5540", timeout=30)
    time.sleep(1)
    assert "Commissioning completed successfully" in open(blog).read()
    assert bp.poll() is None
    ok("Commissioned")

    # ── Static reads ──────────────────────────────────────────────────────
    log("TEST", "Read vendor ID (EP0)...")
    o = ct("basicinformation", "read", "vendor-id", NODE_ID, "0")
    ok("Vendor ID") if "65521" in o else fail("Vendor ID", o)

    log("TEST", "Read descriptor (EP1)...")
    o = ct("descriptor", "read", "device-type-list", NODE_ID, "1")
    ok("Bridge descriptor") if "DeviceTypeList" in o else fail("Descriptor", o)

    # ── MQTT device ───────────────────────────────────────────────────────
    log("TEST", "Creating MQTT dimmer...")
    mqtt("DimmerFeit/AABBCCDDEEFF/1/get", "1"); time.sleep(3)
    b = open(blog).read()
    m = re.search(r'Adding device (\d+)', b)
    ep = str(3 + int(m.group(1))) if m else "4"
    ok(f"MQTT device (EP{ep})") if "Added device" in b else fail("MQTT device", "not in log")

    log("TEST", f"Read OnOff (EP{ep})...")
    o = ct("onoff", "read", "on-off", NODE_ID, ep)
    ok("Read OnOff") if "OnOff:" in o else fail("Read OnOff", o)

    log("TEST", f"Read NodeLabel (EP{ep})...")
    o = ct("bridgeddevicebasicinformation", "read", "node-label", NODE_ID, ep)
    ok("Read NodeLabel") if "NodeLabel:" in o else fail("NodeLabel", o)

    log("TEST", "MQTT OFF → Matter...")
    mqtt("DimmerFeit/AABBCCDDEEFF/1/get", "0"); time.sleep(2)
    o = ct("onoff", "read", "on-off", NODE_ID, ep)
    ok("MQTT OFF reflected") if "FALSE" in o else fail("MQTT OFF", o)

    log("TEST", "Matter ON command...")
    o = ct("onoff", "on", NODE_ID, ep)
    ok("OnOff ON cmd") if "Status" in o else fail("OnOff ON", o)

    log("TEST", f"Read LevelControl (EP{ep})...")
    o = ct("levelcontrol", "read", "current-level", NODE_ID, ep)
    ok("LevelControl") if "CurrentLevel:" in o else fail("LevelControl", o)

    log("TEST", "Second MQTT device...")
    mqtt("DimmerFeit/112233445566/1/get", "1"); time.sleep(3)
    ok(f"Second device") if open(blog).read().count("Added device") >= 2 else fail("Second device", "")

finally:
    if bp and bp.poll() is None: bp.terminate(); bp.wait(timeout=5)
    shutil.rmtree(test_dir, ignore_errors=True)

out(""); out("═" * 40)
out(f"  Results: {G}{passed} passed{N}, {R}{failed} failed{N} (of {passed+failed})")
out("═" * 40); rf.close()
sys.exit(failed)
