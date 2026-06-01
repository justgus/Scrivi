#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"

echo "==> Configuring ScriviCore (Unix Makefiles)..."
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSCRIVI_BUILD_TESTS=ON

echo "==> Building ScriviCore..."
cmake --build "$BUILD_DIR" --parallel

echo "==> Running ctest..."
ctest --test-dir "$BUILD_DIR" --output-on-failure

echo "==> Running swift test..."
cd "$REPO_ROOT/platforms/apple"
swift test

echo "==> All steps passed."
