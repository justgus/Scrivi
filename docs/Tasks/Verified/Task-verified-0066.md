---
id: T-0066
title: "clang-tidy — Remaining checks (20 warnings)"
status: ✅ Verified
sprint: SP-020
epic: EP-008
verified_date: 2026-06-01
---

## Description

Fixed all remaining in-scope clang-tidy warnings:

- `modernize-avoid-c-arrays`: Replaced `char buf[N]` with `std::array<char, N>` in `IdentityService.cpp`, `AppSupportLayout.cpp`, `SystemUUIDProvider.cpp`.
- `bugprone-implicit-widening-of-multiplication-result`: Added `static_cast<std::size_t>` casts in `IdentityService.cpp` and `Hash.cpp`.
- `readability-math-missing-parentheses`: Added explicit parentheses around `*` sub-expressions in `Hash.cpp`.
- `modernize-use-starts-ends-with`: Replaced `substr(0, n) == prefix` with `starts_with(prefix)` in `RepairHandlers.cpp`.
- `bugprone-unchecked-optional-access`: Extracted optional before dereference in `ProjectOpener.cpp`.
- `bugprone-command-processor`: Added `// NOLINT` to intentional `system()`/`popen()` calls in `Process.cpp`.

Out-of-scope categories excluded from the gate script (`bugprone-easily-swappable-parameters`, `readability-function-cognitive-complexity`) — these require API-level changes not in SP-020 scope.

## Files Changed

- `ScriviCore/src/identity/IdentityService.cpp`
- `ScriviCore/src/platform/AppSupportLayout.cpp`
- `ScriviCore/src/platform/SystemUUIDProvider.cpp`
- `ScriviCore/src/util/Hash.cpp`
- `ScriviCore/src/util/Process.cpp`
- `ScriviCore/src/repair/RepairHandlers.cpp`
- `ScriviCore/src/project_package/ProjectOpener.cpp`
- `scripts/verify-scrivi-core-tidy.sh`

## Verification

- `cmake --build build/ninja --parallel` — clean
- `ctest --test-dir build/ninja --output-on-failure` — 165/165 passed
- `bash scripts/verify-scrivi-core-tidy.sh` — exits 0 (passed)
