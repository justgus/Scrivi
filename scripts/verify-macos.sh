#!/usr/bin/env bash
# Full macOS verification: ScriviCore debug build + ctest + swift test + release build.
#
# Optional additional checks (run separately):
#   ./scripts/verify-scrivi-core-asan.sh   — ASan + UBSan
#   ./scripts/verify-scrivi-core-tidy.sh   — clang-tidy (requires: brew install llvm)
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"

echo "==> Configuring ScriviCore (Debug)..."
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSCRIVI_BUILD_TESTS=ON

echo "==> Building..."
cmake --build "$BUILD_DIR" --parallel

echo "==> Running ctest..."
ctest --test-dir "$BUILD_DIR" --output-on-failure

echo "==> Running swift test..."
(cd "$REPO_ROOT/platforms/apple" && swift test)

echo "==> Verifying Release build..."
"$REPO_ROOT/scripts/verify-scrivi-core-release.sh"

echo "==> All steps passed."
