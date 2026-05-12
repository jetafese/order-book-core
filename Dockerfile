FROM seahorn/seahorn-llvm14:nightly

USER root

RUN apt-get update && apt-get install -y \
build-essential \
clang \
cmake \
pkg-config \
libssl-dev \
sudo \
&& rm -rf /var/lib/apt/lists/*

ENV CC=clang-14
ENV CXX=clang++-14

RUN echo "usea ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
USER usea
WORKDIR /home/usea

# install the just command runner
# create /home/usea/bin
RUN mkdir -p bin
# download and extract just to /home/usea/bin/just
RUN curl --proto '=https' --tlsv1.2 -sSf https://just.systems/install.sh | bash -s -- --to bin
# add `/home/usea/bin` to the paths that your shell searches for executables
ENV PATH="/home/usea/bin:$PATH"

WORKDIR /home/usea/
COPY --chown=usea . order-book-core
WORKDIR /home/usea/order-book-core/
RUN git checkout main
