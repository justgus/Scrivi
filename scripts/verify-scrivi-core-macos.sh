#!/usr/bin/env bash
# Validates ScriviCore (C++ only) on macOS using the existing Unix Makefiles build dir.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"

echo "==> Configuring ScriviCore..."
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSCRIVI_BUILD_TESTS=ON

echo "==> Building..."
cmake --build "$BUILD_DIR" --parallel

echo "==> Testing..."
ctest --test-dir "$BUILD_DIR" --output-on-failure

echo "==> ScriviCore macOS: all tests passed."
