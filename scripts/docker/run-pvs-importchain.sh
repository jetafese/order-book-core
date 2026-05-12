#!/usr/bin/env bash
set -euo pipefail

WORKSPACE=${WORKSPACE:-/workspace}
PVS_HOME=${PVS_HOME:-/opt/PVS}
PVS=${PVS:-${PVS_HOME}/pvs}
YICES_BIN=${YICES_BIN:-/opt/yices2/bin/yices}
LOG_DIR=${LOG_DIR:-${WORKSPACE}/build/logs}
LOG=${LOG:-${LOG_DIR}/pvs-importchain.log}

mkdir -p "$LOG_DIR"

if [[ ! -x "$PVS" ]]; then
  printf 'PVS executable not found: %s\n' "$PVS" >&2
  exit 1
fi

if [[ ! -x "$YICES_BIN" ]]; then
  printf 'Yices2 executable not found: %s\n' "$YICES_BIN" >&2
  exit 1
fi

printf 'PVS executable: %s\n' "$PVS"
printf 'PVS yices2 setting: (setq *yices2-executable* "%s")\n' "$YICES_BIN"
printf 'PVS_LIBRARY_PATH: %s\n' "${PVS_LIBRARY_PATH:-}"

top_file="${WORKSPACE}/pvs/top.pvs"
pvs_expr="(setq *yices2-executable* \"${YICES_BIN}\") (tc \"${top_file}\" t) (prove-importchain \"top\" t) (uiop:quit)"

set +e
(
  cd "${WORKSPACE}/pvs"
  "$PVS" -raw -E "$pvs_expr"
) >"$LOG" 2>&1
pvs_status=$?
set -e

if ! grep -q 'Grand Totals:' "$LOG"; then
  printf 'PVS did not produce proof totals. Tail of %s:\n' "$LOG" >&2
  tail -80 "$LOG" >&2
  exit 1
fi

awk '
  /Proof summary for theory/ { printing = 1 }
  printing { print }
  /^Grand Totals:/ { printing = 0 }
' "$LOG"

if grep -Eq 'Error running Yices2|not compiled with mcsat|Typecheck error|No resolution' "$LOG"; then
  printf 'PVS reported a solver/typecheck problem. See %s\n' "$LOG" >&2
  exit 1
fi

if [[ "$pvs_status" -ne 0 ]]; then
  printf 'PVS exited with status %s. See %s\n' "$pvs_status" "$LOG" >&2
  exit "$pvs_status"
fi

if [[ "${CHECK_EXPECTED_PVS_TOTALS:-1}" == "1" ]]; then
  if ! grep -Eq '^Grand Totals: 37 proofs, 37 attempted, 36 succeeded ' "$LOG"; then
    printf 'Unexpected PVS totals. Expected 37 attempted / 36 succeeded. See %s\n' "$LOG" >&2
    exit 1
  fi

  if ! grep -Fq 'exchangeV10WithoutPriceErrorThresholds_TCC1...unfinished' "$LOG"; then
    printf 'Expected unfinished TCC was not present in proof summary. See %s\n' "$LOG" >&2
    exit 1
  fi
fi

printf 'Full PVS log: %s\n' "$LOG"
