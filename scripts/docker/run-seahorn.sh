#!/usr/bin/env bash
set -euo pipefail

WORKSPACE=${WORKSPACE:-/workspace}
SEAHORN_SRC=${SEAHORN_SRC:-/opt/seahorn-src}
SEAHORN_BUILD=${SEAHORN_BUILD:-${SEAHORN_SRC}/build}
SEA=${SEA:-/opt/seahorn/bin/sea}
LOG_DIR=${LOG_DIR:-${WORKSPACE}/build/logs}

mkdir -p "$LOG_DIR"

if [[ ! -x "$SEA" ]]; then
  printf 'SeaHorn executable not found: %s\n' "$SEA" >&2
  exit 1
fi

printf 'SeaHorn executable: %s\n' "$SEA"
SEA_BINDIR=$(dirname "$SEA")
"$SEA" -h | sed -n '1,4p' || true

failures=0

run_build_target() {
  local target=$1
  local log="${LOG_DIR}/seahorn-${target}.log"

  set +e
  cmake --build "$SEAHORN_BUILD" --target "$target" >"$log" 2>&1
  local status=$?
  set -e

  if [[ "$status" -eq 0 ]]; then
    printf 'SeaHorn build/test target %-18s PASS\n' "$target"
  else
    printf 'SeaHorn build/test target %-18s FAIL (%s)\n' "$target" "$status" >&2
    tail -80 "$log" >&2
    failures=$((failures + 1))
  fi
}

has_build_target() {
  local target=$1
  local help_log="${LOG_DIR}/seahorn-targets.log"

  cmake --build "$SEAHORN_BUILD" --target help >"$help_log" 2>&1
  grep -Eq "(^|[[:space:]])${target}($|:|[[:space:]])" "$help_log"
}

run_optional_build_target() {
  local target=$1

  if has_build_target "$target"; then
    run_build_target "$target"
  else
    printf 'SeaHorn build/test target %-18s SKIPPED (not available)\n' "$target"
  fi
}

run_build_target units_z3
run_build_target units_yices2
run_build_target test_type_checker
run_build_target test_hex_dump

for target in ${SEAHORN_TEST_TARGETS:-}; do
  run_optional_build_target "$target"
done

if [[ "${RUN_SEAHORN_LIT:-0}" == "1" ]]; then
  lit_log="${LOG_DIR}/seahorn-lit.log"
  lit_root="${SEAHORN_LIT_ROOT:-${SEAHORN_SRC}/test/simple}"
  if [[ -d "$lit_root" ]]; then
    set +e
    (cd "${SEAHORN_BUILD}/test" && lit --path="$SEA_BINDIR" -sv -v "$lit_root") >"$lit_log" 2>&1
    lit_status=$?
    set -e
    if [[ "$lit_status" -eq 0 ]]; then
      printf 'SeaHorn lit suite PASS\n'
    else
      printf 'SeaHorn lit suite FAIL (%s)\n' "$lit_status" >&2
      tail -120 "$lit_log" >&2
      failures=$((failures + 1))
    fi
  else
    printf 'SeaHorn lit suite SKIPPED (%s not found)\n' "$lit_root"
  fi
else
  printf 'SeaHorn lit suite SKIPPED (RUN_SEAHORN_LIT=%s)\n' "${RUN_SEAHORN_LIT:-0}"
fi

summarize_sea_log() {
  local log=$1
  grep -E '^(sat|unsat|unknown)$|BRUNCH_STAT|Counterexample|__VERIFIER_error|unsat|sat|unknown' "$log" | tail -80 || tail -80 "$log"
}

project_log="${LOG_DIR}/seahorn-project-harness.log"
project_cex="${LOG_DIR}/seahorn-project-harness.cex.ll"
set +e
"$SEA" pf --cex="$project_cex" ${SEA_PF_FLAGS:--m64 -O0 --horn-stats --inline} \
  -I"${WORKSPACE}/exchange" \
  "${WORKSPACE}/exchange/verify.cpp" \
  "${WORKSPACE}/exchange/vOfferExchange.cpp" \
  >"$project_log" 2>&1
project_status=$?
set -e

printf '\nSeaHorn project harness status:\n'
summarize_sea_log "$project_log"
if grep -Eq '^sat$|BRUNCH_STAT Result FALSE|BRUNCH_STAT Result SAT' "$project_log"; then
  printf 'SeaHorn project harness reported UNSAFE/counterexample. See %s\n' "$project_log"
  if [[ -s "$project_cex" ]]; then
    printf 'Project harness counterexample file: %s\n' "$project_cex"
  fi
elif grep -Eq '^unsat$|BRUNCH_STAT Result TRUE|BRUNCH_STAT Result UNSAT' "$project_log"; then
  printf 'SeaHorn project harness reported SAFE.\n'
elif grep -Eq '^unknown$|BRUNCH_STAT Result UNKNOWN' "$project_log"; then
  printf 'SeaHorn project harness reported UNKNOWN. See %s\n' "$project_log"
else
  printf 'SeaHorn project harness did not report sat/unsat/unknown. Command status %s; see %s\n' "$project_status" "$project_log" >&2
  failures=$((failures + 1))
fi

tcc_log="${LOG_DIR}/seahorn-tcc-counterexample.log"
tcc_cex="${LOG_DIR}/seahorn-tcc-counterexample.cex.ll"
set +e
"$SEA" pf --cex="$tcc_cex" ${SEA_PF_FLAGS:--m64 -O0 --horn-stats --inline} \
  "${WORKSPACE}/docker/seahorn/verify_exchange_v10_tcc.cpp" \
  >"$tcc_log" 2>&1
tcc_status=$?
set -e

printf '\nSeaHorn TCC counterexample status:\n'
printf 'TCC counterexample input for ExchangeV10.exchangeV10WithoutPriceErrorThresholds_TCC1:\n'
printf '  price=(3/2), maxWheatSend=0, maxWheatReceive=1, maxSheepSend=1, maxSheepReceive=1, round=PATH_PAYMENT_STRICT_SEND\n'
printf '  computed pre-threshold state: wheatStays=false, wR=0, sS=0\n'
printf '  violated wf_state? conjunct: wR=0 and PATH_PAYMENT_STRICT_SEND implies sS != 0\n'
summarize_sea_log "$tcc_log"

if grep -Eq '^sat$|BRUNCH_STAT Result FALSE|BRUNCH_STAT Result SAT' "$tcc_log"; then
  printf 'SeaHorn reproduced the TCC counterexample.\n'
  if [[ -s "$tcc_cex" ]]; then
    printf 'Counterexample file: %s\n' "$tcc_cex"
    sed -n '1,80p' "$tcc_cex"
  fi
elif grep -Eq '^unsat$|BRUNCH_STAT Result TRUE|BRUNCH_STAT Result UNSAT' "$tcc_log"; then
  printf 'SeaHorn proved the TCC harness safe; expected a counterexample. See %s\n' "$tcc_log" >&2
  failures=$((failures + 1))
else
  printf 'SeaHorn TCC harness did not report sat/unsat. Command status %s; see %s\n' "$tcc_status" "$tcc_log" >&2
  failures=$((failures + 1))
fi

if [[ "$failures" -ne 0 ]]; then
  exit 1
fi
