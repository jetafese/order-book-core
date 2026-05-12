#!/usr/bin/env bash
set -euo pipefail

WORKSPACE=${WORKSPACE:-/workspace}
LOG_DIR=${LOG_DIR:-${WORKSPACE}/build/logs}
BUILD_DIR=${BUILD_DIR:-${WORKSPACE}/build/docker}
BIN=${BIN:-${BUILD_DIR}/exchange-v10-path-send-counterexample}
LOG=${LOG:-${LOG_DIR}/unit-counterexample.log}

mkdir -p "$LOG_DIR" "$BUILD_DIR"

c++ -std=c++17 -Wall -Wextra \
  -I"${WORKSPACE}/exchange" \
  "${WORKSPACE}/docker/counterexamples/exchange_v10_path_send_counterexample.cpp" \
  "${WORKSPACE}/exchange/OfferExchange.cpp" \
  -o "$BIN"

set +e
"$BIN" >"$LOG" 2>&1
status=$?
set -e

cat "$LOG"

if [[ "$status" -ne 0 ]]; then
  printf 'Unit counterexample runner exited with %s. See %s\n' "$status" "$LOG" >&2
  exit "$status"
fi

if ! grep -q 'invalid amount of sheep sent' "$LOG"; then
  printf 'Expected counterexample exception was not observed. See %s\n' "$LOG" >&2
  exit 1
fi

printf 'Unit counterexample reproduced for ExchangeV10.exchangeV10WithoutPriceErrorThresholds_TCC1.\n'

