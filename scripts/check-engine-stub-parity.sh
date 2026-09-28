#!/usr/bin/env bash
# [I-0253] — keep the visionOS ScriviEngine STUB in step with the real engine.
#
# ⚠️ WHY THIS EXISTS. `ScriviEngine.swift` is `#if os(macOS) || os(iOS)` for the
# real implementation and `#else` for a visionOS stub whose every method throws.
# ⛔ When a method is added to the engine and not to the stub, NOTHING notices:
#   • macOS and iOS build fine — they never compile the stub;
#   • Apple CI is LINT-ONLY by user ruling (Q2, 2026-09-21), so no xcodebuild runs;
#   • the visionOS target is not built routinely by anyone.
# ⚠️ The failure surfaces much later, as a compile error in a file that has
# nothing to do with the change that caused it.
#
# ⚠️ IT HAS HAPPENED THREE TIMES, AND THE STUB SAID SO ITSELF. Its own comment
# warned: "the three widened entry points were never widened here, so a
# world-scoped call site would fail to compile on visionOS alone" and "a stub
# that lags the engine breaks only visionOS, long after the change that caused
# it." ⛔ It then drifted twice more:
#   • 56cc2cc — closeProject, openSceneForBulkLoad  → BROKE the visionOS build
#   • 6f287ca — mergeScene, mergeChapter            → latent (macOS-only callers)
# ✅ A comment is not a guard. This is the guard.
#
# ⚠️ WHAT IT CHECKS: every `public func` in the real engine has a same-named
# `public func` in the stub. ⛔ It does NOT compare signatures — that needs a
# parser, and the failure mode being prevented is an ABSENT method, not a
# mistyped one. ✅ A mistyped one fails the visionOS build loudly at the call
# site; an absent one is what hides.
#
# Run: scripts/check-engine-stub-parity.sh     (exit 1 on drift)

set -uo pipefail
cd "$(dirname "$0")/.."

status=0

check_file() {
    local file="$1"
    [ -f "$file" ] || { echo "⚠️  $file not found — skipping"; return; }

    # Locate the #else that opens the stub. ⚠️ Both files use the same shape:
    # `#if os(macOS) || os(iOS)` … `#else` … `#endif` at column 0.
    local else_line
    else_line=$(grep -n '^#else' "$file" | head -1 | cut -d: -f1)
    if [ -z "$else_line" ]; then
        echo "⚠️  $file has no top-level '#else' — no stub to compare. Skipping."
        return
    fi

    local real stub missing
    # ⚠️ TWO THINGS THIS PATTERN HAS TO GET RIGHT, AND IT GOT BOTH WRONG FIRST:
    #
    # 1. `public` is REQUIRED, not optional. ⛔ A bare `func` matched `decodeC` —
    #    a FILE-PRIVATE generic helper at top level, which the stub has no reason
    #    to mirror. ✅ Only the PUBLIC surface can break another target.
    #
    # 2. ⛔ ATTRIBUTES MAY PRECEDE `public`. The first version anchored on
    #    whitespace-then-`public`, so it MISSED every
    #    `@discardableResult public func …` in the stub and reported four methods
    #    as absent THAT WERE ALREADY THERE. ⚠️ Acting on that false positive added
    #    duplicates and BROKE the visionOS build — the very build this guard
    #    exists to protect. ✅ Now any leading attributes are skipped.
    #
    # ⚠️ THE LESSON IS THE GUARD'S OWN: a check that reports a phantom is worse
    # than no check, because it invites a "fix" that breaks something real.
    local pat='^[[:space:]]*(@[a-zA-Z]+[[:space:]]+)*public func [a-zA-Z_][a-zA-Z0-9_]*'
    real=$(awk -v n="$else_line" 'NR < n' "$file" \
           | grep -oE "$pat" | awk '{print $NF}' | sort -u)
    stub=$(awk -v n="$else_line" 'NR > n' "$file" \
           | grep -oE "$pat" | awk '{print $NF}' | sort -u)

    missing=$(comm -23 <(echo "$real") <(echo "$stub"))

    if [ -n "$missing" ]; then
        echo "❌ [$file] the visionOS stub is MISSING methods the real engine has:"
        echo "$missing" | sed 's/^/     • /'
        echo
        echo "   Add each to the stub below the '#else', mirroring the real signature."
        echo "   A method that cannot work off-platform throws:"
        echo "       public func foo(...) throws -> FooResult { try unavailable() }"
        echo "   A non-throwing teardown method is a no-op:"
        echo "       public func closeProject(projectRootPath: String) { }"
        status=1
    else
        echo "✅ [$file] stub parity — every engine method has a stub."
    fi
}

check_file "Scrivi/Engine/ScriviEngine.swift"
check_file "Scrivi/Engine/ScriviEngineGraph.swift"

if [ "$status" -eq 0 ]; then
    echo "✅ Engine stub parity clean."
fi
exit "$status"
