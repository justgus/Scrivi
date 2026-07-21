FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    cmake \
    ninja-build \
    git \
    ca-certificates \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work

CMD ["/bin/bash", "-lc", "cmake -S . -B build/linux-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DSCRIVI_BUILD_TESTS=ON && cmake --build build/linux-debug && ctest --test-dir build/linux-debug --output-on-failure"]
