#!/usr/bin/env python3
import os, json, subprocess, sys

root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
with open(os.path.join(root, 'scenario.json')) as f:
    s = json.load(f)

sim_bin = os.path.join(root, 'build', 'simulator')
if not os.path.exists(sim_bin):
    print("ERROR: simulator binary not found. Build first (mkdir build && cd build && cmake .. && make).")
    sys.exit(1)

os.makedirs(os.path.join(root, 'results'), exist_ok=True)
out_file = os.path.join(root, s.get("out", "results/out.csv"))

cmd = [
    sim_bin,
    "--tasks", os.path.join(root, s.get("tasks_file", "tasks.csv")),
    "--freq", str(int(s.get("freq_hz", 1e9))),
    "--ipc", str(s.get("ipc", 1.0)),
    "--alpha", str(s.get("alpha", 1e-9)),
    "--gamma", str(s.get("gamma", 1.0)),
    "--p_idle", str(s.get("p_idle", 0.5)),
    "--out", out_file
]

print("Running:", " ".join(cmd))
proc = subprocess.run(cmd)
if proc.returncode != 0:
    print("Simulator failed with code", proc.returncode)
    sys.exit(proc.returncode)
print("Wrote results to:", out_file)
