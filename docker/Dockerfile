# syntax=docker/dockerfile:1.6
#
# Ubuntu Jammy development image for the order-book-core PVS/SeaHorn workflow.
# Use the build node's native Linux architecture. This avoids running SBCL under
# x86_64 emulation on Apple Silicon, which can crash with SIGILL during the PVS
# Quicklisp bootstrap.

ARG BASE_IMAGE=ubuntu:22.04
FROM --platform=$BUILDPLATFORM ${BASE_IMAGE}

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

ARG LIBPOLY_REPO=https://github.com/SRI-CSL/libpoly.git
ARG LIBPOLY_REF=master
ARG CUDD_REPO=https://github.com/ivmai/cudd.git
ARG CUDD_REF=cudd-3.0.0
ARG YICES_REPO=https://github.com/SRI-CSL/yices2.git
ARG YICES_REF=Yices-2.6.4
ARG PVS_REPO=https://github.com/SRI-CSL/PVS.git
ARG PVS_REF=master
ARG PVSLIB_REPO=https://github.com/nasa/pvslib.git
ARG PVSLIB_REF=master
ARG SEAHORN_REPO=https://github.com/seahorn/seahorn.git
ARG SEAHORN_REF=main

ARG SRC_DIR=/opt/src
ARG YICES_HOME=/opt/yices2
ARG PVS_HOME=/opt/PVS
ARG SEAHORN_SRC=/opt/seahorn-src
ARG SEAHORN_HOME=/opt/seahorn
ARG Z3_ROOT=/usr
ARG LLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm
ARG LLVM_APT_REPO="deb [signed-by=/usr/share/keyrings/apt.llvm.org.gpg] https://apt.llvm.org/jammy/ llvm-toolchain-jammy-14 main"

ENV DEBIAN_FRONTEND=noninteractive
ENV SRC_DIR=${SRC_DIR}
ENV YICES_HOME=${YICES_HOME}
ENV PVS_HOME=${PVS_HOME}
ENV SEAHORN_SRC=${SEAHORN_SRC}
ENV SEAHORN_BUILD=${SEAHORN_SRC}/build
ENV SEAHORN_HOME=${SEAHORN_HOME}
ENV PVS_LIBRARY_PATH=${PVS_HOME}/nasalib
ENV LD_LIBRARY_PATH=/usr/local/lib:${YICES_HOME}/lib
ENV PATH=${YICES_HOME}/bin:${PVS_HOME}:${SEAHORN_HOME}/bin:/usr/lib/llvm-14/bin:${PATH}

USER root

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      ca-certificates \
      curl \
      gnupg && \
    curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | \
      gpg --dearmor -o /usr/share/keyrings/apt.llvm.org.gpg && \
    echo "${LLVM_APT_REPO}" > /etc/apt/sources.list.d/llvm-toolchain-jammy-14.list && \
    rm -rf /var/lib/apt/lists/*

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      autoconf \
      automake \
      bison \
      build-essential \
      ca-certificates \
      clang-14 \
      clang-tools-14 \
      cmake \
      curl \
      flex \
      g++ \
      gcc \
      git \
      graphviz \
      gperf \
      libboost-all-dev \
      libclang-14-dev \
      libedit-dev \
      libffi-dev \
      libgmp-dev \
      libgraphviz-dev \
      libncurses-dev \
      libreadline-dev \
      libssl-dev \
      libtool \
      libxml2-dev \
      libz3-dev \
      lld-14 \
      llvm-14 \
      llvm-14-dev \
      llvm-14-runtime \
      llvm-14-tools \
      m4 \
      make \
      ninja-build \
      pkg-config \
      python3 \
      python3-pip \
      sbcl \
      uuid-dev \
      z3 \
      zlib1g-dev && \
    apt-get download libpolly-14-dev && \
    dpkg --force-all -i libpolly-14-dev_*.deb && \
    rm -f libpolly-14-dev_*.deb && \
    python3 -m pip install --no-cache-dir lit OutputCheck networkx pygraphviz && \
    uname -m && \
    sbcl --version && \
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p "${SRC_DIR}" && \
    git clone "${LIBPOLY_REPO}" "${SRC_DIR}/libpoly" && \
    cd "${SRC_DIR}/libpoly" && \
    git checkout "${LIBPOLY_REF}" && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    cmake --build build --parallel "$(nproc)" && \
    cmake --install build

RUN git clone "${CUDD_REPO}" "${SRC_DIR}/cudd" && \
    cd "${SRC_DIR}/cudd" && \
    git checkout "${CUDD_REF}" && \
    autoreconf -fi && \
    ./configure --prefix=/usr/local --enable-shared && \
    make -j"$(nproc)" && \
    make install && \
    ldconfig

RUN git clone "${YICES_REPO}" "${SRC_DIR}/yices2" && \
    cd "${SRC_DIR}/yices2" && \
    git checkout "${YICES_REF}" && \
    autoconf && \
    CPPFLAGS="-I/usr/local/include" \
    LDFLAGS="-L/usr/local/lib" \
    LD_LIBRARY_PATH="/usr/local/lib:${LD_LIBRARY_PATH}" \
      ./configure --prefix="${YICES_HOME}" --enable-mcsat && \
    grep -E '^ENABLE_MCSAT *= *yes' configs/make.include.*

RUN cd "${SRC_DIR}/yices2" && \
    make -j"$(nproc)" && \
    make install && \
    rm -f "${YICES_HOME}/lib/libyices.a" && \
    test -f "${YICES_HOME}/lib/libyices.so" && \
    ldconfig

RUN \
    printf '(define x::real)\n(assert (> (* x x) 2))\n(check)\n' > /tmp/yices-mcsat-smoke.ys && \
    "${YICES_HOME}/bin/yices" --mcsat --logic=QF_UFNIRA /tmp/yices-mcsat-smoke.ys | grep -E '^(sat|unknown)$'

RUN git clone "${PVS_REPO}" "${PVS_HOME}" && \
    cd "${PVS_HOME}" && \
    git checkout "${PVS_REF}" && \
    ./configure

RUN cd "${PVS_HOME}" && \
    make -j1

RUN git clone "${PVSLIB_REPO}" "${PVS_HOME}/nasalib" && \
    cd "${PVS_HOME}/nasalib" && \
    git checkout "${PVSLIB_REF}" && \
    printf '(setq *yices2-executable* "%s/bin/yices")\n' "${YICES_HOME}" > /root/.pvs.lisp

RUN git clone "${SEAHORN_REPO}" "${SEAHORN_SRC}" && \
    cd "${SEAHORN_SRC}" && \
    git checkout "${SEAHORN_REF}" && \
    mkdir -p build && \
    cd build && \
    z3_lib="/usr/lib/$(gcc -dumpmachine)/libz3.so" && \
    test -f "$z3_lib" && \
    cmake .. -GNinja \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_CXX_COMPILER=clang++-14 \
      -DCMAKE_C_COMPILER=clang-14 \
      -DSEA_ENABLE_LLD=ON \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_INSTALL_PREFIX="${SEAHORN_HOME}" \
      -DYICES2_HOME="${YICES_HOME}" \
      -DLLVM_DIR="${LLVM_DIR}" \
      -DZ3_INCLUDE_DIR=/usr/include \
      -DZ3_LIBRARY="$z3_lib" \
      -DZ3_ROOT="${Z3_ROOT}" && \
    cmake --build . --target extra && \
    cmake .. && \
    cmake --build . --target crab && \
    cmake .. && \
    cmake --build . --target install && \
    ln -sf "$(command -v clang-14)" "${SEAHORN_HOME}/bin/clang" && \
    ln -sf "$(command -v clang++-14)" "${SEAHORN_HOME}/bin/clang++"

WORKDIR /workspace
COPY . /workspace
RUN chmod +x /workspace/scripts/docker/*.sh

CMD ["/workspace/scripts/docker/run-all.sh"]
