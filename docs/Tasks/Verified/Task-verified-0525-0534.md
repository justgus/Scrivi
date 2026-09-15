# Verified Tasks — T-0525, T-0526, T-0527, T-0532, T-0534

**Sprint:** [SP-133](../../Sprints/Sprint-active.md) — `[Apple]` TextKit 2
**Epic:** [EP-039](../../Epics/Epic-active.md) — `[Cross]` Project Load Performance
**Verified:** 2026-09-14 (user-approved) · **Archived:** 2026-09-15 at the EP-039 close

⚠️ **NOT all of SP-133's tasks are here.** ✅ **Only the five the user VERIFIED.**
⚠️ **T-0528 / T-0530 / T-0531 reached a conclusion but were never user-verified; ⚠️ T-0529 is
HALF-verified** (✅ the live window drag, ⛔ not the `rebuildStorage` split/merge case) —
✅ **all four stay in the Sprint record rather than being archived as done.**

| ID | Task | Status |
| -- | ---- | ------ |
| **T-0525** | ✅ **Replace the TWO TextKit 1 call sites.** ⚠️ **`ManuscriptTextView.swift:541`** (scroll handler: `glyphIndex(for:)` + `characterIndexForGlyph`) → `textLayoutFragment(for:)`; ⚠️ **`:1591`** (`centerStorageOffset`: `glyphRange` + `boundingRect`) → fragment geometry. ⚠️ **These two lines are the ENTIRE downgrade** — ✅ **touching `.layoutManager` at all forces TextKit 1** | **High** | 🔵 Not started | 🟡 **Active** | 🟢 **Implemented - Not Verified** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED)** — ✅ **the two TextKit 1 call sites replaced**; ⚠️ **console confirms `TextKit 2` at construction AND at `rebuildStorage`.**
| **T-0526** | ✅ **Port the scene divider off `NSTextAttachmentCell`** → `NSTextAttachmentViewProvider` / `attachmentBoundsForAttributes`. ⚠️ **`NSTextAttachmentCell` is TextKit 1 AND AppKit-ONLY — it has NO UIKit equivalent**, ✅ **so it blocks iOS independently of performance.** ✅ **The replacement is `macos(12.0), ios(15.0)`** | **High** | 🔵 Not started | 🟡 **Active** | 🟢 **Implemented - Not Verified** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED)** — ✅ **the divider ported off `NSTextAttachmentCell`**; ⚠️ **console confirms `TextKit 2` at construction AND at `rebuildStorage`.**
| **T-0527** | ✅ **A GUARD so the downgrade cannot return** (5.7 ruling). ⚠️ **A CI grep that FAILS on `.layoutManager` / `NSLayoutManager` / `NSTextAttachmentCell` in `Scrivi/Views`**, ✅ **with an allow-list carrying reasons.** ⚠️ **MUST LAND WITH THE MIGRATION, not after** — ⚠️ **one future access silently re-downgrades the whole view** | **High** | 🔵 Not started | 🟡 **Active** | 🟢 **Implemented - Not Verified** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED)** — ✅ **the CI downgrade guard**; ⚠️ **console confirms `TextKit 2` at construction AND at `rebuildStorage`.**
| **T-0532** | ✅ **[I-0204] — CACHE THE TIMELINE'S DERIVED BOUNDS AND KILL THE QUADRATIC CLUSTER SORT.** ✅ **MEASURED via `sample Scrivi` during a live stall (2026-09-14): 4,796 / 6,595 main-thread samples — 73% — in `TimelineStripView.buildClusters`.** ⚠️ **ZERO in the Scene Navigator; TextKit barely registers.** ✅ **THREE FIXES:** ✅ **(a) `minOffsetMs`/`maxEndMs`/`spanMs` cached on `TimelineViewModel` via `didSet` on `dots`+`historicalEvents` (`:202`, `:213`, `recomputeDerivedBounds()` `:249`);** ✅ **(b) `smallestMainRowGapMs` moved to the model and cached the same way — it BUILT AND SORTED a 1,156-element array per call, and `maxZoom` calls it (`:682`);** ✅ **(c) decorate-sort-undecorate in `buildClusters` (`:1390`) — `itemX` was called INSIDE the sort comparator, ~23,000 calls → 1,156.** ⚠️ **This SUPERSEDES the belief that the remaining half of [I-0200] was TextKit** | ⚠️ **HIGH** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED).** ⚠️ **PRIOR:** 🟢 **Implemented - Not Verified — ✅ CONFIRMED BY LIVE RUN 2026-09-14.** ⚠️ **`NSClickGestureRecognizer … possible phase ~33 s` ABSENT from the whole session**; ✅ **click `35–45 s` → `~0.3 s`; user reports the app USABLE.** ⚠️ **Awaiting USER verification** |
| **T-0534** | ✅ **[I-0207] half (c) — STOP `reconnectWorlds` RUNNING ONCE PER WINDOW.** ⚠️ **The `didBecomeActive` observer lived on `ManuscriptEditorView` (PER WINDOW) while `reconnectWorlds()` loops EVERY session — ✅ N windows × N sessions = N² session-reloads per activation.** ⚠️ **Same duplication on the `didMount`/`didUnmount` observers beside it.** ✅ **FIX: `AppEnvironment.coalescedReconnectWorlds()`; all three observers call it.** ⚠️ **COALESCING, NOT MERELY ONE SUBSCRIBER — `didBecomeActive` is legitimately delivered more than once per activation (✅ the sample shows it BOTH nested in `_NXFinishActivation` AND standalone), so a single subscriber would still double-run.** ⚠️ **A FLAG, NOT A DEBOUNCE: a timer would delay world re-acquisition, which is exactly what [I-0123] exists to prevent.** ---- ⚠️ **RECORDED: my first reading of the sample was WRONG** — ✅ **I claimed two distinct `sendEvent:` offsets proved two activations; they are the SAME offset (`+2232`), one nested in the other.** | ⚠️ **HIGH** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED).** ⚠️ **THE PRIOR STATUS LINE WAS STALE AND IS CORRECTED:** ⚠️ **it said *"half (b) (off-main-thread) is still not done"*** — ✅ **half (b) HAD ALREADY LANDED with [T-0523]'s scope extension.** ✅ **VERIFIED IN THE CODE, not from the record: `WorldWarningModel.reloadAsync` runs the sweep on `Task.detached(priority: .utility)` (`WorldWarningView.swift:73`) and publishes on the main actor; ✅ its ONLY callers are `AppEnvironment.swift:518` and `EditorView.swift:113`, both `await`ing it** — ⛔ **NO synchronous `reload` caller remains.** ✅ **[I-0207] is VERIFIED AND ARCHIVED with ALL THREE halves confirmed by the user's live pass: (a) the binding cache [T-0533], (b) off-main-thread [T-0523], (c) the coalescing [T-0534].** ✅ **The focus-click beachball is GONE.** |

### What these delivered

✅ **T-0525/T-0526/T-0527 — the TextKit 2 migration itself.** ⚠️ **Two `layoutManager` call sites were
the ENTIRE downgrade;** ✅ **the console now confirms `TextKit 2` at construction AND at every
`rebuildStorage`.** ✅ **T-0527's CI guard landed WITH the migration so a single future
`.layoutManager` access cannot silently re-downgrade the view.**

✅ **T-0532 — the timeline's derived-bounds cache and the quadratic cluster sort.** ⚠️ **MEASURED by
`sample` at 73% of main-thread samples in `buildClusters`;** ✅ **the 33 s gesture stall is gone from
the whole session and clicks went `35–45 s` → `~0.3 s`.**

✅ **T-0534 — `coalescedReconnectWorlds()`.** ⚠️ **A PER-WINDOW observer ran an ALL-WINDOWS loop:
N windows x N sessions = N² session reloads per activation.** ✅ **Coalescing, not debouncing —
a timer would delay world re-acquisition, which is what [I-0123] exists to prevent.**
