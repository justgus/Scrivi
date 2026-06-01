---
id: SP-020
title: "clang-tidy Housekeeping — Full Warning Sweep"
status: ✅ Closed
epic: EP-008
start_date: 2026-06-01
end_date: 2026-06-01
---

## Goal

Eliminate all actionable clang-tidy warnings across ScriviCore's 42 source files. Warnings were grouped into four tasks by category. Gate: `scripts/verify-scrivi-core-tidy.sh` must pass with zero warnings, and all 165 ctests must remain green.

## Assigned Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0063 | clang-tidy — `readability-braces-around-statements` (142 warnings) | ✅ Verified |
| T-0064 | clang-tidy — `modernize-use-designated-initializers` + `readability-qualified-auto` (51 warnings) | ✅ Verified |
| T-0065 | clang-tidy — `[[nodiscard]]`, `const`, and `static` member functions (21 warnings) | ✅ Verified |
| T-0066 | clang-tidy — Remaining checks (20 warnings) | ✅ Verified |

## Assigned Issues

None.

## Retrospective

**What went well:**
- The mechanical nature of braces and designated-initializer fixes made T-0063 and T-0064 fast to execute once the pattern was clear.
- The `verify-scrivi-core-tidy.sh` script (running clang-tidy directly on ScriviCore sources, not through CMake) correctly isolated ScriviCore from third-party Catch2 code — this was the right approach.
- 165/165 tests remained green throughout every step.

**What to watch:**
- `bugprone-easily-swappable-parameters` (18 instances) and `readability-function-cognitive-complexity` (5 instances) were intentionally excluded from the gate — they require API-level changes. These are candidates for a future housekeeping sprint if the API stabilizes.
- The partial `--fix` run from an earlier attempt corrupted several files (malformed brace placement, invalid `static [[nodiscard]]` ordering). Manual rewrite of `RepairHandlers.cpp` was required. Always prefer manual edits over automated `--fix` for complex files.

## Final State

- `bash scripts/verify-scrivi-core-tidy.sh` — exits 0
- `ctest --test-dir build/ninja --output-on-failure` — 165/165 passed
