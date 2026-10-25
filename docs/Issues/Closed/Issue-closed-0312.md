# Closed Issue (Not Verified) — I-0312

## I-0312: `[ScriviCore]` `keyBetween` sorts a new key before an undotted legacy key

**Title:** `keyBetween` gives a key that sorts before an undotted multi-character legacy key it should follow
**Status:** ⚪ **CLOSED — Not Verified (2026-10-24, user-directed): OBE**
**Reason for Closure:** the user, 2026-10-24: *"I-0312 … do[es] not apply to anyone as there are no official projects that
are not using the new scheme. We should retire these Issues."* Only a project from before EP-027 can hold the undotted
keys (`001`–`009`, `011` …), and no such project is in use. If one turns up, reopen this record.
**Platform:** `[ScriviCore]`
**Component:** `util/OrderKey` (`keyBetween`), and every caller that places a scene or chapter between neighbours
**Severity:** Medium — a new scene can land in the wrong place, but only next to a legacy three-digit key; nothing is lost
**Sprint:** Not Assigned
**Date Identified:** 2026-10-20 (testing [SP-186]'s `scrivi_range_paste`; a probe of `keyBetween` itself)

**What happens (measured, `keyBetween` called directly):**

| lo | hi | result | sorts |
| -- | -- | ------ | ----- |
| `004` | `005` | `0.0.4.H` | ⛔ before `004` (`'.'` < `'0'`) |
| `001` | `002` | `0.0.1.H` | ⛔ before `001` |
| `002` | `003` | `0.0.2.H` | ⛔ before `002` |
| `001` | `""` | `A` | ✅ |
| `001` | `A` | `5` | ✅ |

**Cause (read, not yet proven by a fix):** `keyBetween` parses a key into digits and renders the result in its DOTTED form
(`fromDigits`: *"first digit plain, a dot before each rest"*). An undotted legacy key like `004` parses as the digits
0, 0, 4, but the dotted rendering of those digits is `0.0.4`, which is not the same string, and the scene and chapter
listings sort keys as plain bytes (`SceneIndex.cpp:92`, `ChapterIndex.cpp:100`).
**Who is exposed:** any key between two undotted multi-character keys. `migrateScenes` keeps `001`–`009`, `011`… as valid
order keys (only a key ending in `0` is reslugged), so a project from before EP-027 can still hold them, and every project's
first scene is `001-opening-scene`. `SceneCreator` and `ChapterCreator` call `keyBetween` the same way the range
operations do.
⚠️ Not yet reproduced in the app. To check: split a scene that sits between two legacy-keyed scenes (`002`, `003`) and see
where the new scene lands.
**Test fixtures:** `RangeOperationBudgetTests.cpp` and `RangeOperationCApiTests.cpp` write generated-shape keys (`B`, `C`, …)
as the app does, and say why.
