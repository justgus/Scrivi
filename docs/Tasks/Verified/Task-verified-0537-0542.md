# Verified Tasks — T-0537, T-0542

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0537** | ⚠️ **Retire Linux's duplicate `InspectorLayoutStore`** — ✅ **route it through [SP-141]'s core endpoints** | [SP-142] | [EP-041] | **2026-09-21** |
| **T-0542** | ⚠️ **[I-0241]** — ✅ **project `tags` from the core; ⛔ DELETE Linux's historical-event disk walk** | [SP-142] | [EP-041] | **2026-09-21** |

✅ **Both VERIFIED by the user's live pass on BOTH Apple and Ubuntu, 2026-09-21.**

---

## T-0537 — one schema, one owner

⚠️ **`scrivi.inspector-layout.v1` had TWO app-side owners and they had already drifted** ([I-0215]:
Linux preserved unknown keys, Apple dropped them). ✅ **[SP-141] built the core endpoints and converted
Apple; T-0537 converted Linux.**

- ✅ **`ScriviBridge` gained `getInspectorLayout` / `putInspectorLayout`.**
- ✅ **`InspectorLayoutStore` GUTTED** — ⛔ **zero `QFile`/`QSaveFile`/`QDir`.** ✅ **The class was KEPT
  and hollowed (user ruling Q2, "keep it, gut it"): it still owns MEANING** — the defaults, the tab
  vocabulary, and what to do about an `unreadable` document.
- ✅ **All three `status` values handled.** ⚠️ **`unreadable` keeps `loaded_ = false`, so no setter can
  overwrite a damaged file** — ✅ **pre-existing behaviour, preserved deliberately.**
- ✅ **The smoke test was REPOINTED, ⛔ its assertions UNCHANGED** — ⚠️ **they were always about the
  SCHEMA's behaviour, never about the Qt class.** ✅ **Its CMake target gained `ScriviBridge` +
  `ScriviCore`, which it had never linked.**

---

## T-0542 — ⚠️ **the filed Issue understated it: there were TWO bugs**

✅ **[I-0241] said "list drops tags." True — and not the whole defect.**

1. ⛔ **`listHistoricalEvents` DROPPED `tags`.** ⚠️ **The loop had already read and parsed every event
   file; the data was in hand and thrown away.** ✅ **Now projected.**
2. ⛔ **`create`/`update` NEVER STORED THEM EITHER.** ⚠️ **Both called `getStringArray("tags")` on the
   parsed payload — which reads an array held UNDER a key, while `scrivi.h` documented a ROOT array.**
   ✅ **So the DOCUMENTED shape could never work.** ⚠️ **A dead `arraySize("tags")` block sat beside it,
   showing the gap was noticed and left.**

⚠️ **AND THE OBVIOUS FIX WOULD HAVE BROKEN LINUX.** ⛔ **Linux sends `{"tags":[…]}` (`tagsToJson`),
which the OLD BUGGY CODE happened to read correctly** — ✅ **so narrowing the parse to the documented
root array would have silently broken the only live producer.** ✅ **Both shapes now accepted through
ONE helper, each asserted, and `scrivi.h` corrected to document both.**
✅ **Added `JsonDoc::rootStringArray()`** — ⚠️ **the accessor whose absence caused bug 2.**

✅ **`readHistoricalEventTagsFromDisk` DELETED.** ⚠️ **It listed and parsed EVERY file in
`objects/historical-events/` to recover ONE field** — ⛔ **unbounded work per lookup**
(`project_read_amplification_class`) — ✅ **and a TOMBSTONE marks the site.**

⚠️ **ITS SECOND CALLER WAS A DATA-LOSS PATH.** ✅ **`onHistoricalEventDragged` re-read tags only to
re-send them** (`updateHistoricalEvent` overwrites all fields), ⛔ **and the disk read returned an
EMPTY list on EVERY failure** — ⚠️ **so a failed read SILENTLY ERASED that event's tags.**
✅ **Tags now come from the list projection via `histEvents_`.**

---

## How they were verified

✅ **USER LIVE PASS, 2026-09-21, BOTH PLATFORMS — all checks passed:**
- ✅ **the inspector tab survived a quit and relaunch on Ubuntu;**
- ✅ **THE MAC CROSS-CHECK: the same project opened on Apple with its card layout INTACT** —
  ⚠️ **the half no suite can report on, and precisely the failure [I-0215] was;**
- ✅ **a historical event WITH tags was DRAGGED and its tags SURVIVED** (⚠️ **checked separately from
  the Edit-dialog prefill** — `feedback_verify_each_half_separately`).

**Automated:**
| Check | Result |
| ----- | ------ |
| `ctest` macOS | ✅ **626/626** (was 621 — 5 new ABI tests) |
| `ctest` **Linux, NON-ROOT, tests ON** | ✅ **630/630** — ⚠️ **a SECOND image** (`project_linux_container_tests_off`) |
| Linux Docker app build | ✅ **318/318 targets, ZERO warnings** |
| Linux smokes | ✅ **24/24** |
| `xcodebuild` build / test | ✅ **clean / 132/132** |

⚠️ **THREE INJECTED-DEFECT PROOFS — ⛔ a green test that cannot fail is not evidence:**
1. ✅ **Core projection removed → 3 of 5 ABI tests fail.**
2. ✅ **Linux store made to RECONSTRUCT instead of patch (the [I-0215] shape) → 9 of 17 smoke checks
   fail**, ⚠️ **naming `stackSort`, `scenes`, `defaultStacks` and the unknown key** — ✅ **exactly what
   a writer would lose.**
3. ✅ **Drag path, projection removed → both new assertions fail**, ⚠️ **including *"tags SURVIVED the
   drag."***

---

## ⚠️ What the live pass found that the Sprint did not plan for

✅ **[I-0242]** — ⚠️ **filed the same day.** ⛔ **Linux NEVER READS the writer's card stack.**
✅ **`inspector-layout.json` holds `defaultStacks` and per-scene `scenes`; T-0537 PROVED Linux
round-trips both losslessly** — ⛔ **and `InspectorLayoutStore` exposes only `selectedTab`.**

⚠️ **So this Task made Linux a faithful custodian of a document it does not consult** —
✅ **`project_capability_without_surface`, found the day the custody was proven.**
✅ **User ruling: the writer chooses which cards show and in what order; an empty card still renders
with a "No objects" message, as Apple already does.** ✅ **→ [EP-036] AC4a/AC4b.**

---

*Verified by the user 2026-09-21 after a live pass on both platforms. Implemented under [SP-142] / [EP-041].*
