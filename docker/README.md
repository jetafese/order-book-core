# Docker Verification Workflow

Build the Ubuntu Jammy development image:

```sh
docker build -f docker/Dockerfile -t order-book-core-dev .
```

On Apple Silicon, do not force `linux/amd64`; SBCL can crash under x86_64
emulation while PVS bootstraps Quicklisp. The Dockerfile asks Docker to use the
builder's native Linux architecture.

Run the whole verification bundle:

```sh
docker run --rm order-book-core-dev
```

The default entrypoint runs:

1. PVS typechecking for `/workspace/pvs/top.pvs`, then `(prove-importchain "top" t)`.
2. A deterministic C++ unit counterexample for the still-unproved `ExchangeV10.exchangeV10WithoutPriceErrorThresholds_TCC1` obligation.
3. SeaHorn build smoke tests, the existing project harness status, and a deterministic SeaHorn counterexample for the same TCC-shaped `wf_state?` obligation.

Useful focused runs:

```sh
docker run --rm order-book-core-dev /workspace/scripts/docker/run-pvs-importchain.sh
docker run --rm order-book-core-dev /workspace/scripts/docker/run-unit-counterexample.sh
docker run --rm order-book-core-dev /workspace/scripts/docker/run-seahorn.sh
```

The image builds Yices2 `Yices-2.6.4` from GitHub with `--enable-mcsat` after installing libpoly and CUDD from GitHub. That pin keeps SeaHorn on the older Yices status-enum API it currently expects. The static `libyices.a` is removed after install so SeaHorn links `libyices.so`, which carries the libpoly/CUDD dependencies needed by MCSAT. PVS is configured with:

```lisp
(setq *yices2-executable* "/opt/yices2/bin/yices")
```

The SeaHorn runner always runs the stable solver/type-checker smoke targets. The existing full project harness is reported as `SAFE`, `UNSAFE`, or `UNKNOWN`; `UNKNOWN` is treated as a status, not a failure. The dedicated TCC harness directly encodes the concrete `ExchangeV10.exchangeV10WithoutPriceErrorThresholds_TCC1` `wf_state?` violation and is expected to report `sat`/`FALSE`.

The larger upstream lit suites are optional because several are architecture-sensitive on native arm64 builds. Set `SEAHORN_TEST_TARGETS="test-simple test-solve"` to run CMake lit targets, or `RUN_SEAHORN_LIT=1` to run lit directly. The proof harnesses omit `-g` by default to avoid LLVM debug metadata verifier crashes on SeaHorn/LLVM 14; override with `SEA_PF_FLAGS` if you want different `sea pf` flags.
