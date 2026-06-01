#!/usr/bin/env bash
# Runs clang-tidy over ScriviCore source files.
# Requires: brew install llvm
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# Prefer Homebrew LLVM clang-tidy over Apple's (which doesn't ship clang-tidy).
LLVM_TIDY="$(brew --prefix llvm 2>/dev/null)/bin/clang-tidy"
if [ -x "$LLVM_TIDY" ]; then
    export PATH="$(brew --prefix llvm)/bin:$PATH"
elif ! command -v clang-tidy &>/dev/null; then
    echo "ERROR: clang-tidy not found. Install it with: brew install llvm" >&2
    exit 1
fi

echo "==> clang-tidy: $(clang-tidy --version | head -1)"

# Ensure compile_commands.json exists (ninja debug build produces it).
COMPILE_DB="$REPO_ROOT/build/ninja"
if [ ! -f "$COMPILE_DB/compile_commands.json" ]; then
    echo "==> Configuring ScriviCore (ninja debug, for compile_commands.json)..."
    cmake -S "$REPO_ROOT" -B "$COMPILE_DB" \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DSCRIVI_BUILD_TESTS=ON \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build "$COMPILE_DB" --parallel
fi

CHECKS="-*,modernize-*,readability-*,bugprone-*,performance-*,clang-analyzer-*"
CHECKS="$CHECKS,-modernize-use-trailing-return-type"
CHECKS="$CHECKS,-readability-identifier-length"
CHECKS="$CHECKS,-readability-magic-numbers"
CHECKS="$CHECKS,-readability-named-parameter"
CHECKS="$CHECKS,-bugprone-easily-swappable-parameters"
CHECKS="$CHECKS,-readability-function-cognitive-complexity"

SOURCES=$(find "$REPO_ROOT/ScriviCore/src" -name "*.cpp" | sort)

echo "==> Running clang-tidy over $(echo "$SOURCES" | wc -l | tr -d ' ') source files..."
WARNINGS=0
while IFS= read -r src; do
    output=$(clang-tidy -p "$COMPILE_DB" --checks="$CHECKS" \
        --header-filter="$REPO_ROOT/ScriviCore/(include|src)/.*" \
        "$src" 2>&1)
    if echo "$output" | grep -q "warning:"; then
        echo "$output" | grep "warning:"
        WARNINGS=1
    fi
done <<< "$SOURCES"

if [ "$WARNINGS" -ne 0 ]; then
    echo "ERROR: clang-tidy found warnings." >&2
    exit 1
fi

echo "==> ScriviCore clang-tidy: passed."
