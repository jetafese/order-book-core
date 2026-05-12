#!/usr/bin/env bash
set -euo pipefail

failures=0

run_step() {
  local label=$1
  shift

  printf '\n== %s ==\n' "$label"
  if "$@"; then
    printf '== %s: PASS ==\n' "$label"
  else
    local status=$?
    printf '== %s: FAIL (%s) ==\n' "$label" "$status" >&2
    failures=$((failures + 1))
  fi
}

run_step "PVS import-chain proofs" /workspace/scripts/docker/run-pvs-importchain.sh
run_step "Unit counterexample" /workspace/scripts/docker/run-unit-counterexample.sh
run_step "SeaHorn statuses and counterexample" /workspace/scripts/docker/run-seahorn.sh

if [[ "$failures" -ne 0 ]]; then
  printf '\nVerification bundle finished with %s failing step(s).\n' "$failures" >&2
  exit 1
fi

printf '\nVerification bundle finished successfully.\n'

