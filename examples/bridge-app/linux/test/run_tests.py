#!/usr/bin/env python3
"""Regression tests for the Matter Linux Bridge.

Uses chip-tool interactive mode with batched commands for speed.
Commissions via direct IP (no mDNS needed).
"""

import subprocess
import os
import sys
import time
import re
import select
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
    detail = detail[:200] if len(detail) > 200 else detail
    _out(f"{RED}[FAIL]{NC} {msg}: {detail}")


def strip_ansi(t):
    return re.sub(r'\x1b\[[0-9;]*m', '', t).replace('\x1b[0J', '')


def mqtt_pub(topic, payload):
    subprocess.run(["mosquitto_pub", "-t", topic, "-m", payload],
                   capture_output=True, timeout=5)


class ChipToolInteractive:
    """Manages a chip-tool interactive session."""
    def __init__(self, storage_dir):
        self.proc = subprocess.Popen(
            [CHIP_TOOL, "interactive", "start", "--storage-directory", storage_dir],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, bufsize=1
        )
        time.sleep(3)
        self._drain()

    def _drain(self):
        out = ''
        while select.select([self.proc.stdout], [], [], 0.2)[0]:
            out += self.proc.stdout.readline()
        return strip_ansi(out)

    def send(self, cmd, wait=5):
        """Send command and return all [TOO] data lines."""
        self._drain()  # clear pending
        self.proc.stdin.write(cmd + '\n')
        self.proc.stdin.flush()
        deadline = time.time() + wait
        out = ''
        while time.time() < deadline:
            time.sleep(0.3)
            out += self._drain()
            # Check if we got data (not just Sending/Command lines)
            data = [l for l in out.splitlines()
                    if '[TOO]' in l and 'Sending' not in l and 'Command:' not in l
                    and 'cluster' not in l and 'ReadAttribute' not in l]
            if data:
                time.sleep(0.5)  # let any trailing output arrive
                out += self._drain()
                break
        return out

    def close(self):
        try:
            self.proc.stdin.write('exit\n')
            self.proc.stdin.flush()
            self.proc.terminate()
            self.proc.wait(timeout=5)
        except Exception:
            self.proc.kill()


# ── Preflight ─────────────────────────────────────────────────────────────────

log("SETUP", "Checking prerequisites...")
for path, name in [(BRIDGE_BIN, "bridge"), (CHIP_TOOL, "chip-tool")]:
    if not os.path.isfile(path):
        _out(f"Missing {name}: {path}")
        sys.exit(1)

r = subprocess.run(["pgrep", "-x", "mosquitto"], capture_output=True)
if r.returncode != 0:
    _out("mosquitto not running")
    sys.exit(1)

subprocess.run(["pkill", "-f", "chip-bridge-app"], capture_output=True)
time.sleep(1)

# ── Setup ─────────────────────────────────────────────────────────────────────

test_dir = tempfile.mkdtemp(prefix="bridge-test-")
tool_storage = os.path.join(test_dir, "ts")
os.makedirs(tool_storage)

for conf in ["mqttDeviceNames.conf", "pingDevices.conf"]:
    src = os.path.join(BRIDGE_DIR, conf)
    if os.path.exists(src):
        shutil.copy(src, test_dir)

bridge_log_path = os.path.join(test_dir, "bridge.log")
bridge_proc = None
ct = None

try:
    # ── Start bridge ──────────────────────────────────────────────────────

    log("SETUP", "Starting bridge...")
    bridge_log = open(bridge_log_path, "w")
    bridge_proc = subprocess.Popen(
        [BRIDGE_BIN, "--KVS", os.path.join(test_dir, "kvs")],
        stdout=bridge_log, stderr=bridge_log, cwd=test_dir
    )
    time.sleep(4)
    if bridge_proc.poll() is not None:
        fail_test("Bridge startup", "died")
        sys.exit(1)
    with open(bridge_log_path) as f:
        if "Server Listening" not in f.read():
            fail_test("Bridge startup", "not listening")
            sys.exit(1)
    pass_test("Bridge started and listening")

    # ── Commission via direct IP ──────────────────────────────────────────

    log("TEST", "Commissioning via direct IP...")
    r = subprocess.run(
        [CHIP_TOOL, "pairing", "already-discovered", NODE_ID, PASSCODE,
         "127.0.0.1", "5540", "--storage-directory", tool_storage],
        capture_output=True, text=True, timeout=30
    )
    time.sleep(1)
    with open(bridge_log_path) as f:
        blog = f.read()
    if "Commissioning completed successfully" in blog:
        pass_test("Commissioned via IP")
    else:
        fail_test("Commission", strip_ansi(r.stdout + r.stderr)[-200:])
        sys.exit(1)

    # ── Start interactive session ─────────────────────────────────────────

    log("SETUP", "Starting chip-tool interactive session...")
    ct = ChipToolInteractive(tool_storage)

    # Warmup: establish CASE session
    ct.send("basicinformation read vendor-id 1 0", wait=8)

    # ── Read vendor ID (EP0) ──────────────────────────────────────────────

    log("TEST", "Read vendor ID (EP0)...")
    out = ct.send("basicinformation read vendor-id 1 0")
    if "VendorID" in out and "65521" in out:
        pass_test("Vendor ID (0xFFF1)")
    else:
        fail_test("Vendor ID", out[:200])

    # ── Read bridge descriptor (EP1) ──────────────────────────────────────

    log("TEST", "Read descriptor (EP1)...")
    out = ct.send("descriptor read device-type-list 1 1")
    if "DeviceTypeList" in out or "deviceType" in out.lower():
        pass_test("Bridge descriptor (EP1)")
    else:
        fail_test("Bridge descriptor", out[:200])

    # ── MQTT device creation ──────────────────────────────────────────────

    log("TEST", "Creating MQTT dimmer...")
    mqtt_pub("DimmerFeit/AABBCCDDEEFF/1/get", "1")
    time.sleep(3)

    with open(bridge_log_path) as f:
        blog = f.read()

    device_ep = None
    for line in blog.splitlines():
        if "Adding device" in line:
            m = re.search(r'Adding device (\d+)', line)
            if m:
                device_ep = 3 + int(m.group(1))
            break

    if device_ep and "Added device" in blog:
        pass_test(f"MQTT device created (EP{device_ep})")
    else:
        fail_test("MQTT device creation", "not in bridge log")
        device_ep = 4

    ep = str(device_ep)

    # ── Read OnOff ────────────────────────────────────────────────────────

    log("TEST", f"Read OnOff (EP{ep})...")
    out = ct.send(f"onoff read on-off 1 {ep}")
    if "OnOff:" in out:
        pass_test("Read OnOff")
    else:
        fail_test("Read OnOff", out[:200])

    # ── Read NodeLabel ────────────────────────────────────────────────────

    log("TEST", f"Read NodeLabel (EP{ep})...")
    out = ct.send(f"bridgeddevicebasicinformation read node-label 1 {ep}")
    if "NodeLabel:" in out:
        pass_test("Read NodeLabel")
    else:
        fail_test("Read NodeLabel", out[:200])

    # ── MQTT OFF → Matter ─────────────────────────────────────────────────

    log("TEST", "MQTT OFF → Matter...")
    mqtt_pub("DimmerFeit/AABBCCDDEEFF/1/get", "0")
    time.sleep(2)
    out = ct.send(f"onoff read on-off 1 {ep}")
    if "FALSE" in out:
        pass_test("MQTT OFF reflected in Matter")
    else:
        fail_test("MQTT OFF reflected", out[:200])

    # ── Matter ON command ─────────────────────────────────────────────────

    log("TEST", "Matter ON command...")
    out = ct.send(f"onoff on 1 {ep}")
    if "Status" in out or "status" in out:
        pass_test("Matter OnOff ON command")
    else:
        fail_test("Matter OnOff ON", out[:200])

    # ── Read LevelControl ─────────────────────────────────────────────────

    log("TEST", f"Read LevelControl (EP{ep})...")
    out = ct.send(f"levelcontrol read current-level 1 {ep}")
    if "CurrentLevel:" in out:
        pass_test("Read LevelControl")
    else:
        fail_test("Read LevelControl", out[:200])

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
    if ct:
        ct.close()
    if bridge_proc and bridge_proc.poll() is None:
        bridge_proc.terminate()
        try:
            bridge_proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            bridge_proc.kill()
    shutil.rmtree(test_dir, ignore_errors=True)

# ── Summary ───────────────────────────────────────────────────────────────────

_out("")
_out("════════════════════════════════════════")
_out(f"  Results: {GREEN}{passed} passed{NC}, {RED}{failed} failed{NC} (of {passed + failed})")
_out("════════════════════════════════════════")
_result_fh.close()
sys.exit(failed)
