# Active Tasks

## SP-020 — clang-tidy Housekeeping

### T-0063: clang-tidy — `readability-braces-around-statements` (142 warnings)

**Status:** 🟣 Implemented — Not Verified
**Sprint:** SP-020
**Epic:** None
**Priority:** Medium

Add braces around all single-statement `if`/`else`/`for`/`while` bodies across all 42 ScriviCore source files.

---

### T-0064: clang-tidy — `modernize-use-designated-initializers` + `readability-qualified-auto` (51 warnings)

**Status:** 🟣 Implemented — Not Verified
**Sprint:** SP-020
**Epic:** None
**Priority:** Medium

Replace positional struct initializations with designated initializers. Replace unqualified `auto` iterator variables with `auto*` or `const auto*` where appropriate.

---

### T-0065: clang-tidy — `[[nodiscard]]`, `const`, and `static` member functions (21 warnings)

**Status:** 🟣 Implemented — Not Verified
**Sprint:** SP-020
**Epic:** None
**Priority:** Medium

Add `[[nodiscard]]` to query methods. Mark member functions `const` and `static` where appropriate.

---

### T-0066: clang-tidy — Remaining checks (20 warnings)

**Status:** 🟣 Implemented — Not Verified
**Sprint:** SP-020
**Epic:** None
**Priority:** Low

Fix remaining small-count warnings: `modernize-avoid-c-arrays`, `bugprone-implicit-widening`, `bugprone-command-processor`, `readability-math-missing-parentheses`, misc.

---

*Last Updated: 2026-06-01 (T-0063–T-0066 all implemented — not verified)*
