#!/usr/bin/env bash
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
# build
rm -rf build
mkdir -p build
cd build
cmake .. >/dev/null
cmake --build . -- -j >/dev/null
cd "$ROOT"
# run orchestrator
python3 python/orchestrator.py
# compare
cmp --silent results/sample_out.csv tests/expected_mvp.csv && echo "TEST OK: output matches expected" || (echo "TEST FAIL: output differs"; echo "==== produced ===="; cat results/sample_out.csv; echo "==== expected ===="; cat tests/expected_mvp.csv; exit 2)
