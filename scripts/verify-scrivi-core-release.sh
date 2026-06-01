#!/usr/bin/env bash
# Confirms ScriviCore builds cleanly in Release mode (no tests — release has tests OFF).
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "==> Configuring ScriviCore (Release)..."
cmake -S "$REPO_ROOT" -B "$REPO_ROOT/build/ninja-release" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DSCRIVI_BUILD_TESTS=OFF

echo "==> Building..."
cmake --build "$REPO_ROOT/build/ninja-release" --parallel

echo "==> ScriviCore Release build: passed."
