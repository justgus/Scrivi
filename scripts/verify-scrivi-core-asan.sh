#!/usr/bin/env bash
# Runs ScriviCore tests under AddressSanitizer + UndefinedBehaviorSanitizer.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "==> Configuring ScriviCore (ASan + UBSan)..."
cmake -S "$REPO_ROOT" -B "$REPO_ROOT/build/asan-ubsan" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSCRIVI_BUILD_TESTS=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
    -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"

echo "==> Building..."
cmake --build "$REPO_ROOT/build/asan-ubsan" --parallel

echo "==> Testing under ASan + UBSan..."
ctest --test-dir "$REPO_ROOT/build/asan-ubsan" --output-on-failure

echo "==> ScriviCore ASan + UBSan: all tests passed."
