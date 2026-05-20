# Task-verified-0003

## T-0003: Utility Foundation

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** 2026-05-19
**Sprint Assigned:** SP-001

**Rationale:**
Schema readers/writers and service implementations depend on path manipulation, slug generation, text statistics, JSON wrapping, atomic file writes, and content hashing. These utilities must be correct and unit-tested before any schema or service work.

**Current Behavior:**
No utility implementations exist.

**Desired Behavior:**
All utility modules pass unit tests. UTF-8 strings are preserved through round-trips. Slug generation is conservative and deterministic.

**Requirements:**
1. `PathUtils` — path joining, extension handling, relative-to-absolute, UTF-8 at boundary
2. `Slug` — conservative slug generation (lowercase, hyphen-separated, no special chars)
3. `TextStats` — word count, character count from Markdown UTF-8 string
4. `Json` wrapper — thin wrapper around nlohmann/json; must not expose nlohmann types in public headers
5. `AtomicWrite` — write to temp file then rename (atomic on POSIX and Windows)
6. `Hash` — SHA-256 or similar content hash for change detection

**Design Approach:**
Implement in `ScriviCore/src/util/`. Keep headers internal (not in `include/scrivi/`). nlohmann/json is included via CMake FetchContent and used only inside `Json.cpp`. AtomicWrite uses `std::filesystem::rename` which is atomic on most POSIX targets.

**Components Affected:**
- ScriviCore/src/util/: PathUtils.hpp/cpp, Slug.hpp/cpp, TextStats.hpp/cpp, Json.hpp/cpp, AtomicWrite.hpp/cpp, Hash.hpp/cpp

**Implementation Details:**
- `PathUtils`: wraps `std::filesystem` — join, extension, filename, parent, makeAbsolute, replaceExtension
- `Slug`: conservative ASCII-only lowercasing; non-alnum runs → single hyphen; leading/trailing stripped
- `TextStats`: UTF-8 code-point counter (skips continuation bytes) + whitespace-split word counter
- `Json`: pimpl wrapping nlohmann/json — nlohmann types fully hidden behind `JsonDoc` opaque handle; `parseJson()` free function returns `Result<JsonDoc>`
- `AtomicWrite`: writes to `.tmp` sibling then `std::filesystem::rename` into place
- `Hash`: self-contained SHA-256 (no OpenSSL dependency); returns lowercase hex string
- nlohmann/json v3.11.3 added via FetchContent in `ScriviCore/CMakeLists.txt`
- Tests get `src/` on their include path so they can `#include "util/Foo.hpp"`

**Test Steps:**
1. `unit/PathUtilsTests.cpp` — 6 tests: path join, extension, filename, parent, makeAbsolute, replaceExtension ✅
2. `unit/SlugTests.cpp` — 6 tests: basic title, punctuation, separator collapse, leading/trailing, empty, numbers ✅
3. `unit/TextStatsTests.cpp` — 6 tests: simple prose, extra whitespace, empty, ASCII char count, markdown, single word ✅
4. `unit/JsonSchemaTests.cpp` — 5 tests: valid parse, malformed parse error, missing key defaults, round-trip, contains ✅
5. Total: 29/29 tests passing

**Notes:**
The Json wrapper is a key guardrail: nlohmann types must never leak into public headers or service interfaces.

---

*Verified: 2026-05-19*
