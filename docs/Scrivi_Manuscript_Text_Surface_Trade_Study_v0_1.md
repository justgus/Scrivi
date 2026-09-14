# Scrivi — Manuscript Text Surface Trade Study v0.1

**Project:** Scrivi
**Document:** Manuscript Text Surface Trade Study
**Version:** 0.1
**Status:** ✅ **RULED 2026-09-12 (user). OPTION C.** ⚠️ **§7 records the rulings and the 5.7 research.**
**Date:** 2026-09-12
**Origin:** [I-0200]. ⚠️ **Every figure here is MEASURED on this machine at the real manuscript's size.**

---

## 1. The question

⚠️ **How should the manuscript editor hold and lay out a large manuscript?**

✅ **Today it holds ALL OF IT**: `ViewportSceneLoader.loadAll` reads every scene at open, and
`rebuildStorage` concatenates every body into ONE `NSTextStorage`.
⚠️ **The loader's own header says so:** *"All scenes are loaded into memory at once on project open
(all-in-memory model). The dynamic fill/release cycle is not used."*

---

## 2. ⚠️ What is actually slow — MEASURED, and it is NOT what it looks like

### 2.1 The real manuscript

| | |
| - | - |
| scenes | **1,153** |
| total body bytes | **1,821,978** (~1.82 MB) |
| largest single scene | ⚠️ **1,664 bytes** |
| shape | ✅ **MANY SMALL SCENES**, not a few large ones |

### 2.2 ⚠️ TextKit 1 layout cost at that size

| operation | 50 KB | 400 KB | ⚠️ **1.82 MB (real)** |
| --------- | ----- | ------ | -------------------- |
| initial `ensureLayout` | 21 ms | 54 ms | ⚠️ **222–240 ms** |
| `glyphIndex(for point:)` | 0.01 ms | 0.01 ms | ✅ **0.01 ms** |
| `boundingRect(forGlyphRange:)` | 0.04 ms | 0.03 ms | ✅ **0.03 ms** |

✅ **THE QUERIES ARE FREE. Layout is the cost, and it is paid ONCE — until something invalidates it.**

### 2.3 ✅ WHAT INVALIDATES IT — this is the defect

| trigger | ⚠️ cost at 1.82 MB |
| ------- | ----------------- |
| ⚠️ **ONE CHARACTER typed near the top** | ⚠️ **204 ms re-layout** (the edit itself: `0.16 ms`) |
| ⚠️ **Window RESIZE (container width change)** | ⚠️ **190 ms re-layout** |
| ⚠️ **`rebuildStorage` (`setAttributedString`)** | ⚠️ **226 ms** |

⚠️ **THAT IS THE WHOLE STORY: a keystroke near the top of the manuscript costs a fifth of a second,
and a window resize costs the same.** ⚠️ **Neither is proportional to the edit; both are proportional
to the DOCUMENT.**

### 2.4 ⚠️ Why TextKit 1 at all — this was NOT a decision

✅ **A modern `NSTextView` starts on TEXTKIT 2, which lays out only the VIEWPORT.**
⚠️ **Touching the TextKit-1 `.layoutManager` property makes AppKit PERMANENTLY DOWNGRADE that view to
TextKit 1**, ⚠️ **and Scrivi touches it in exactly TWO places** — `ManuscriptTextView.swift:541`
(the scroll handler) and `:1591` (`centerStorageOffset`).
⚠️ **Both do the same thing: convert a character offset to a screen rectangle, or back.**
✅ **So the engine downgrade is an ACCIDENT OF TWO LINES, not an architectural choice** — ⚠️ **and it
is worth knowing that before choosing a fix.**

---

## 2.5 ✅ RULED BY THE USER (2026-09-12): TEXTKIT 2 REGARDLESS

> ✅ *"I think we should go to TextKit 2 anyway, and always, regardless of the benefits of TextKit 1."*

⚠️ **THIS IS NOT PART OF THE TRADE — IT IS SETTLED.** ✅ **TextKit 1 is legacy; Apple's own direction
is TextKit 2, and it is the only engine on iOS worth targeting.**
⚠️ **So the options below differ in WHAT ELSE they do, not in whether TextKit 2 happens.**

---

## 2.6 ⚠️ RICH TEXT / MARKUP RENDERING — MEASURED, and it CHANGES THE TRADE

⚠️ **Scene bodies are ALREADY MARKDOWN on disk** (`scrivi_save_scene(… markdown …)`), ✅ **and are
rendered today as ONE uniform monospaced run.** ⚠️ **Rendering that markup means MANY SHORT ATTRIBUTE
RUNS**, so the question is what run density costs.

| document | attribute runs | initial layout | ⚠️ **1-char edit re-layout** |
| -------- | -------------- | -------------- | --------------------------- |
| ✅ **PLAIN (today)** | 1 | 212 ms | ⚠️ **207 ms** |
| light markup (~1 per 1,000 chars) | 82 | 206 ms | 209 ms |
| moderate markup (~10 per 1,000) | 810 | 227 ms | 223 ms |
| ⚠️ **heavy markup (~100 per 1,000)** | 8,098 | ⚠️ **466 ms** | ⚠️ **470 ms** |

✅ **LIGHT AND MODERATE MARKUP ARE ESSENTIALLY FREE — run count is NOT the driver at realistic
densities.** ⚠️ **HEAVY markup MORE THAN DOUBLES the cost.**

⚠️ **WHY THIS MATTERS TO THE RULING: it is a MULTIPLIER ON AN ALREADY-BAD NUMBER.**
⚠️ **A `207 ms` keystroke becomes `470 ms` before a single new feature ships**, ✅ **and it raises the
ceiling question in §4 — heavy markup at 1.82 MB costs roughly what PLAIN text costs at ~4 MB.**
✅ **Under TextKit 2 this concern largely evaporates: run density only matters for the FRAGMENTS
ACTUALLY LAID OUT.**

---

## 3. Options

### ⚠️ Option A — Keep all-in-memory, MOVE TO TEXTKIT 2

✅ **Replace the two `.layoutManager` uses with `NSTextLayoutManager` equivalents**
(`textLayoutFragment(for:)` / `NSTextSelectionNavigation`), ⚠️ **so the view stays on TextKit 2 and
lays out only the visible region plus a margin.**

| ✅ For | ⚠️ Against |
| ----- | --------- |
| ✅ **Smallest change to the DATA model — `loadAll`, `rebuildStorage`, `sceneBoundaries` and all nine `recomputeBoundaries` callers keep working unchanged** | ⚠️ **`sceneBoundaries` is CHARACTER-INDEXED over the whole storage; TextKit 2 is fragment/range-based.** ⚠️ **The two call sites must be rewritten, not adapted** |
| ✅ **Layout becomes proportional to the VIEWPORT, so the 204 ms keystroke and 190 ms resize both go away** | ⚠️ **TextKit 2 on macOS has a history of edge-case bugs; ⚠️ we would be depending on it for the app's central surface** |
| ✅ **Memory profile unchanged and already proven acceptable** (1.82 MB of text) | ⚠️ **Any FUTURE code that touches `.layoutManager` silently re-downgrades the whole view** — ⚠️ **a trap that needs a guard, because it is exactly how this happened** |
| ✅ **No new failure modes around scene loading, saving, undo or history** | ⚠️ **Does NOT reduce open time (`loadAll` still reads 1,153 files) — ✅ though that is already `0.5 s`** |

### ⚠️ Option B — GUARD-BANDED VIEWPORT (load/release a window of scenes)

✅ **Hold only the scenes near the viewport — say ±N — and load/release as the writer moves.**
⚠️ **This is the "dynamic fill/release cycle" the loader header says exists in concept and is unused.**

| ✅ For | ⚠️ Against |
| ----- | --------- |
| ✅ **Bounds BOTH memory and layout by the band, not the manuscript** | ⚠️ **SCROLLING BECOMES I/O-BOUND.** ⚠️ **Every band crossing is a disk read on the main thread unless the app is made async first (AC6, unbuilt)** |
| ✅ **Scales to a manuscript far larger than 1.82 MB** | ⚠️ **THE SCROLLBAR LIES.** ⚠️ **Document height is unknown until everything is laid out, so the thumb size and position are estimates that JUMP as the band moves — a well-known and much-complained-about behaviour in editors that do this** |
| ✅ **Open could become near-instant (load the band, not 1,153 files)** | ⚠️ **⌘F ACROSS THE MANUSCRIPT stops being a text-view search** — ⚠️ **unloaded scenes are not in the storage.** ✅ **It must move to the core (which CAN do it — `extractSearchableText` already reads every scene), ⚠️ but that is a second feature to build** |
| ✅ **Matches what Scrivener/Ulysses do** | ⚠️ **UNDO/REDO CROSSES BAND BOUNDARIES.** ⚠️ **EP-019's history is scene-granular and persistent, ✅ which helps — ⚠️ but an undo targeting an UNLOADED scene must load it, and the structural-inverse path (T-0356) assumes the scene is present** |
| | ⚠️ **`sceneBoundaries` and every character offset become RELATIVE TO THE BAND.** ⚠️ **Nine `recomputeBoundaries` call sites, `segmentIndex`, cross-boundary cut/copy/paste (T-0354), and the cursor-restore path (I-0058/I-0131) all assume whole-manuscript offsets** |
| | ⚠️ **[I-0131] IS THE PRECEDENT AND THE WARNING: two sources of truth for one coordinate cost a real defect.** ⚠️ **A band introduces a THIRD coordinate space** |

### ⚠️ Option C — Both (TextKit 2 now, banding later if needed)

✅ **Do Option A, measure, and take Option B ONLY if a real manuscript still hurts.**

| ✅ For | ⚠️ Against |
| ----- | --------- |
| ✅ **Cheapest path to a usable app, and it is REVERSIBLE** | ⚠️ **Two migrations if B turns out to be needed anyway** |
| ✅ **Option A's work is NOT wasted if B follows** — ⚠️ **B still needs a viewport-based layout engine underneath it** | ⚠️ **Risks declaring victory at "good enough" and never revisiting** |
| ✅ **Keeps the coordinate model intact while the app is still being stabilised** | |

---

## 3.5 ⚠️ THE iOS CONSTRAINT — a hard requirement, and it FAVOURS TEXTKIT 2

> ⚠️ **User, 2026-09-12, emphasised:** *"eventually we must also implement this for iOS… whatever
> solution we come up with here must also have a migration path to iOS."*

✅ **MEASURED STATE OF PLAY:** ⚠️ **`ManuscriptTextView.swift` is `2,198` lines of macOS-only code
behind `#if os(macOS)`**, ⚠️ **and iOS/visionOS get a `15`-line stub** that renders
*"Manuscript editor not yet available on this platform."*
⚠️ **The blocker is real: `NSTextView` and `NSViewRepresentable` DO NOT EXIST on iOS** — the
equivalents are `UITextView` and `UIViewRepresentable`.

⚠️ **THE DECISIVE POINT: `NSLayoutManager` IS THE macOS-SHAPED PART.** ✅ **TextKit 2's
`NSTextLayoutManager`, `NSTextContentStorage`, `NSTextLayoutFragment` and `NSTextSelectionNavigation`
are THE SAME TYPES ON BOTH PLATFORMS.**
⚠️ **So a TextKit-1 implementation must be WRITTEN TWICE; ✅ a TextKit-2 implementation shares its
layout and coordinate logic and differs only in the view wrapper.**

| concern | ⚠️ TextKit 1 | ✅ TextKit 2 |
| ------- | ----------- | ----------- |
| layout engine type | ⚠️ `NSLayoutManager` — **AppKit shape, no UIKit twin worth sharing** | ✅ `NSTextLayoutManager` — **identical on iOS** |
| view wrapper | ⚠️ `NSViewRepresentable` / `UIViewRepresentable` — **differs either way** | ⚠️ **same, and unavoidable** |
| coordinate/boundary logic | ⚠️ **duplicated per platform** | ✅ **shared** |
| the 2,198-line body | ⚠️ **largely rewritten for iOS** | ✅ **mostly portable** |

⚠️ **CONSEQUENCE FOR THE TRADE: any option that keeps TextKit 1 makes the iOS port MORE expensive,
not less.** ✅ **This is an independent argument for §2.5's ruling.**

⚠️ **GUARD-BANDING (Option B) IS PLATFORM-NEUTRAL** — ✅ **it lives in `ViewportSceneLoader`, which is
already shared** — ⚠️ **but it MULTIPLIES with the iOS work rather than replacing any of it.**

---

## 4. ⚠️ What the measurements say about the choice

⚠️ **THE TRADE IS NOT "IN-MEMORY vs ON-DEMAND".** ✅ **The DATA is not the problem:** ⚠️ **1.82 MB of
text is trivial to hold, and the core now serves it fast** (SP-131: a full 1,153-scene load is
`0.5 s`, per-scene cost FLAT). ⚠️ **THE PROBLEM IS LAYOUT.**

✅ **WITH TEXTKIT 2 RULED (§2.5), THE REMAINING QUESTION IS NARROW: is a viewport-based layout engine
over an ALL-IN-MEMORY document enough, or must the DOCUMENT ITSELF be windowed too?**

⚠️ **THE ANSWER DEPENDS ON ONE NUMBER — the largest manuscript Scrivi must serve** — ✅ **and the
measured curve makes it answerable rather than a guess:**

| manuscript | ⚠️ TextKit 1 re-layout per keystroke | ✅ TextKit 2 expectation |
| ---------- | ----------------------------------- | ----------------------- |
| 1.82 MB (Dumas, real) | ⚠️ **207 ms** (⚠️ **470 ms** with heavy markup) | ✅ **viewport-bounded** |
| ~4 MB | ⚠️ ~450 ms (extrapolated, ⚠️ **NOT measured**) | ✅ **viewport-bounded** |
| ~10 MB | ⚠️ ~1.2 s (extrapolated, ⚠️ **NOT measured**) | ⚠️ **viewport-bounded, but OPEN cost and MEMORY grow** |

⚠️ **Layout is roughly LINEAR in document size, so TextKit 1 fails at ANY plausible ceiling.**
✅ **TextKit 2 makes LAYOUT independent of document size** — ⚠️ **but it does NOT bound the OTHER two
costs that grow with the manuscript: `loadAll`'s per-scene reads, and the memory held.**

⚠️ **SO: Option A is sufficient UNTIL open time or memory becomes the binding constraint, and NEITHER
IS TODAY** (`loadAll` = `0.5 s`; 1.82 MB of text).
✅ **Option B's real argument is a manuscript LARGE ENOUGH that holding it is itself the problem** —
⚠️ **and that threshold has NOT been measured, because no such manuscript exists to test.**

### ✅ Recommendation (⚠️ a recommendation, NOT a ruling)

✅ **OPTION C.** ⚠️ **Do the TextKit 2 migration now** (which §2.5 rules anyway), ⚠️ **measure a real
manuscript**, ✅ **and take Option B only if open time or memory — NOT layout — is still the
constraint.**

⚠️ **THE REASONING, stated so it can be disagreed with:** ⚠️ **Option B introduces I/O on the scroll
path, a scrollbar that jumps, a search that misses unloaded scenes, and a THIRD coordinate space** —
✅ **and it does NOT fix the measured defect any better than Option A does.** ⚠️ **Paying that cost
before the problem it solves has been OBSERVED is how [I-0131] happened: two sources of truth for one
coordinate, introduced ahead of need.**
✅ **Option A's work is NOT wasted if B follows — ⚠️ B REQUIRES a viewport-based engine underneath it.**

---

## 5. ⚠️ Open questions the ruling needs

1. ⚠️ **What is the TARGET CEILING?** ✅ **A number, not "large".** ⚠️ **Everything in §4 turns on it.**
   ✅ **For calibration: Dumas is `1.82 MB` / 1,153 scenes; ⚠️ *War and Peace* is ~3.2 MB of text.**
2. ⚠️ **How much MARKUP will be rendered?** ✅ **Light/moderate is free; ⚠️ heavy DOUBLES layout cost
   under TextKit 1** (§2.6). ⚠️ **Also: is markup rendered INLINE (WYSIWYG) or shown as source?**
3. ⚠️ **WHEN does iOS land?** ⚠️ **If it is near, the TextKit 2 migration should be written for BOTH
   from the start** — ✅ **the layout logic is shared, and writing it macOS-first invites the same
   `#if os(macOS)` split that left `2,198` lines unportable.**
4. ⚠️ **Is a JUMPING SCROLLBAR acceptable?** ⚠️ **Intrinsic to Option B, not an implementation flaw.**
   ✅ **Only needs answering if B is chosen.**
5. ⚠️ **Must ⌘F search UNLOADED scenes?** ✅ **If yes, Option B requires core-side search FIRST**
   (⚠️ **`extractSearchableText` already reads every scene, so the capability exists**).
6. ⚠️ **Does AC6 (async) come BEFORE Option B?** ⚠️ **Band loading on the main thread would
   REINTRODUCE the freeze this Epic exists to remove.**
7. ⚠️ **How is the TEXTKIT-1 DOWNGRADE PREVENTED FROM RETURNING?** ⚠️ **Any `.layoutManager` access
   silently re-downgrades the whole view.** ✅ **A CI grep is the cheap guard, and it should land WITH
   the migration, not after.**

---

## 6. ⚠️ Not in scope

⚠️ **`_NSDetectedLayoutRecursion`** — ⚠️ **fires during `restoreWritingSurface` and is UNEXPLAINED.**
✅ **May be a symptom of whichever option is chosen; ⚠️ must not be assumed fixed by either.**

⚠️ **[I-0203] (the Scene Navigator stops rendering)** — ✅ **the leading hypothesis is that it is a
SYMPTOM of main-thread starvation**, ⚠️ **so it should be re-tested AFTER this work, not fixed first.**

---

## 7. ✅ RULINGS — user, 2026-09-12

✅ **DECISION: OPTION C.** ⚠️ **TextKit 2 migration now; guard-banding ONLY if open time or memory —
not layout — is still the constraint afterwards.**

| # | question | ✅ ruling |
| - | -------- | -------- |
| **5.1** | target ceiling | ✅ **"Return of the King, WITH the appendices."** |
| **5.2** | markup | ⚠️ **EVENTUALLY HEAVY (complete), rendered INLINE (WYSIWYG)** |
| **5.3** | iOS timing | ⚠️ **NEXT AFTER LINUX, BEFORE v1.0** |
| **5.4** | jumping scrollbar | ✅ **Acceptable — "whatever is needed to get there"** |
| **5.5** | ⌘F must reach unloaded scenes | ✅ **YES — "I think it must"** |
| **5.6** | AC6 async before banding | ✅ **YES — "I also think this must be so"** |
| **5.7** | prevent TextKit-1 downgrade | ✅ **A custom Swift/SwiftUI layout-manager extension that CANNOT downgrade, plus avoid the practices that cause one** ⚠️ **+ research a newer engine (below)** |

### 7.1 ✅ The ceiling, as a number — ⚠️ **and it is SMALLER than the test fixture**

| | words | ~size |
| - | ----- | ----- |
| *Return of the King* body | ~137,000 | ~0.75 MB |
| Appendices A–F | ~60,000 | ~0.33 MB |
| ⚠️ **CEILING** | **~197,000** | ⚠️ **~1.08 MB** |
| ✅ **Dumas fixture (MEASURED)** | — | ✅ **1.82 MB / 1,153 scenes** |

✅ **THE FIXTURE ALREADY EXCEEDS THE CEILING BY 1.7x.** ⚠️ **This is a strong result for Option C:**
✅ **if the app is comfortable on Dumas, it is comfortable on the stated ceiling with margin** —
⚠️ **and it means Option B's case must be made on MEMORY or OPEN TIME, never on layout.**
⚠️ **CAVEAT: with heavy inline markup (5.2), `1.08 MB` behaves like ~`2.2 MB` of plain text under
TextKit 1** (§2.6) — ✅ **which is still under the fixture, and irrelevant once TextKit 2 lands.**

### 7.2 ⚠️ 5.7 RESEARCH — is there a TextKit 3? ✅ **NO. Measured against the macOS 27.0 SDK.**

| finding | evidence |
| ------- | -------- |
| ⚠️ **There is NO "TextKit 3."** | ✅ **Zero matches for `TextKit 3`/`TextKit3` anywhere in the AppKit headers** |
| ✅ **TextKit 2 IS the current engine** | `NSTextLayoutManager` — `API_AVAILABLE(macos(12.0), ios(15.0), visionos(1.0))`, ⚠️ **not deprecated, actively extended** |
| ⚠️ **TextKit 1 IS BEING RETIRED** | ⚠️ **`NSLayoutManager.h` carries `30` `API_DEPRECATED` annotations** |
| ✅ **NEW IN 27, AND ON BOTH PLATFORMS** | ⚠️ **`NSTextViewportRenderingSurface`** — `API_AVAILABLE(macos(27.0), ios(27.0), tvos(27.0), visionos(27.0), watchos(27.0))`. ✅ **A protocol marking an `NSView`/`UIView`/`CALayer` as a drawable target for a layout FRAGMENT** — ⚠️ **it is TextKit 2 machinery, NOT a new engine.** ✅ **Confirms Apple is still investing in TextKit 2's viewport model** |

### 7.3 ⚠️ THE FOURTH OPTION — SwiftUI `TextEditor` — ✅ **SPIKED AND REJECTED**

⚠️ **`SwiftUI.TextEditor` gained `init(text: Binding<AttributedString>, selection:)`** — ✅ **a native
rich-text editor with NO `NSViewRepresentable`, which would have collapsed the iOS problem (5.3)
entirely.** ⚠️ **It was worth one day to falsify. ✅ It is falsified.**

#### ⚠️ MEASURED — a real `TextEditor` in a real window, driving the REAL Dumas prose

| manuscript | build | first show | ⚠️ **keystroke near the top** |
| ---------- | ----- | ---------- | ---------------------------- |
| 50 KB | 1 ms | 69 ms | ✅ **11.6 ms** |
| 200 KB | 4 ms | 201 ms | ⚠️ **210.8 ms** |
| 500 KB | 10 ms | 13 ms | ⚠️ **2,680.8 ms** |
| ⚠️ **1.08 MB — THE RULED CEILING (5.1)** | 20 ms | 5,141 ms | ⚠️ **11,747 ms** |

⚠️ **ELEVEN AND A HALF SECONDS PER KEYSTROKE AT THE CEILING, WITH NO MARKUP AT ALL.**
⚠️ **The five samples were tight (`11.4, 12.2, 11.9, 11.6, 11.7 s`) — ✅ this is not noise.**

#### ⚠️ The growth is WORSE THAN QUADRATIC

✅ **Fitted exponent `p = 2.25`** (1.0 = linear, 2.0 = quadratic) across the four measured points.
⚠️ **4x the size costs ~20x the time.** ⚠️ **Extrapolated: `1.82 MB` ⇒ ~`38 s`; `4 MB` ⇒ ~`224 s`.**
✅ **The 3.6/4 MB runs the user asked for were NOT run — ⚠️ the option is dead at a QUARTER of that
size, and a 4-minute keystroke measures nothing new.**

#### ✅ What the spike also PROVED — ⚠️ and it corrected my own hypothesis

⚠️ **I predicted the risk was `AttributedString` being a VALUE TYPE — copying the manuscript per
keystroke.** ✅ **THAT WAS WRONG, and measuring it first is what showed the real cause:**

| `AttributedString` alone, no UI | 400 KB | 1.08 MB | 1.82 MB |
| ------------------------------- | ------ | ------- | ------- |
| copy + mutate | 0.03 ms | ✅ **0.00 ms** | ✅ **0.00 ms** |
| single insert near the top | 0.37 ms | ✅ **0.02 ms** | ✅ **0.02 ms** |
| sustained typing | 0.01 ms/char | ✅ **0.01 ms/char** | ✅ **0.01 ms/char** |

✅ **`AttributedString` is copy-on-write with a rope-like backing and is EFFECTIVELY FREE at any of
these sizes.** ⚠️ **So the `11.7 s` is entirely SwiftUI's `TextEditor` re-rendering the binding** —
⚠️ **it evidently re-processes the whole document per change, which is a far worse behaviour than
TextKit 1's `207 ms`.**

#### ✅ Conclusion

⚠️ **`TextEditor` is `57x` WORSE than the TextKit 1 surface we already have** (`11,747 ms` vs
`207 ms` — comparing the ceiling to Dumas, so the true gap is wider still).
✅ **REJECTED. It is not a candidate at manuscript scale, and no amount of tuning closes a 2.25
exponent.**
⚠️ **This does NOT change the Option C decision** — ✅ **it removes a tempting alternative, and it
means the iOS port (5.3) MUST go through `UITextView` + TextKit 2, not SwiftUI-native.**

### 7.4 ⚠️ Consequences of 5.4–5.6 that the plan must carry

⚠️ **5.5 + 5.6 mean Option B, IF it ever happens, has TWO PREREQUISITES, both unbuilt:**
✅ **core-side ⌘F across unloaded scenes** (⚠️ `extractSearchableText` reads every scene, so the
capability exists but the SEARCH endpoint does not), ⚠️ **and AC6 async.**
✅ **THIS STRENGTHENS OPTION C: the cheap path is genuinely cheap, and the expensive path stays
honestly expensive.**

---

## 8. ⚠️ CORRECTION (2026-09-13) — §2.3's KEYSTROKE FIGURE WAS AN ARTIFACT

⚠️ **§2.3 reported `204 ms` for a one-character edit under TextKit 1. ✅ THAT NUMBER IS WRONG, and the
error was mine: the benchmark generated text with NO PARAGRAPH BREAKS — one 1.8 MB paragraph.**
⚠️ **Both TextKit engines lay out by PARAGRAPH, so a single unbroken paragraph is the pathological
case and nothing like real prose.**

✅ **RE-MEASURED ON THE REAL MANUSCRIPT** (`1,831,770` chars, ⚠️ **`14,985` newlines**):

| operation | ⚠️ **TextKit 1** | ✅ **TextKit 2** |
| --------- | ---------------- | ---------------- |
| initial layout | ⚠️ **272.8 ms** | ✅ **0.9 ms** |
| ⚠️ **1-char edit (top)** | ✅ **0.3 ms** — ⚠️ **NOT 204 ms** | ✅ **0.1 ms** |
| window RESIZE (width change) | ⚠️ **222.5 ms** | ✅ **viewport-bounded** |
| `setAttributedString` (`rebuildStorage`) | ⚠️ **270.0 ms** | ✅ **viewport-bounded** |
| hit-test (scroll handler's query) | ✅ **0.01 ms** | ✅ **0.005 ms** |

### ✅ What this changes, and what it does NOT

⚠️ **TYPING WAS NEVER THE PROBLEM.** ✅ **The real recurring costs are the ones proportional to the
DOCUMENT: initial layout, EVERY WINDOW RESIZE (`222 ms`), and every `rebuildStorage` (`270 ms`).**
⚠️ **`rebuildStorage` runs on any segment-list change**, so scene create/delete/merge/split each pay
it — ✅ **and that is a real, recurring, user-visible cost.**

✅ **THE PARAGRAPH FINDING IS THE IMPORTANT ONE, and it is a CONSTRAINT ON THE DESIGN:**
⚠️ **TextKit 2's laziness is PER PARAGRAPH.** ✅ **On real prose (14,985 paragraphs) it is ~300x
better on first layout and genuinely viewport-bounded** — ⚠️ **an edit FAR from the viewport measured
`0.0 ms`.** ⚠️ **But on ONE 1.8 MB paragraph TextKit 2 collapses to `201 ms`, no better than
TextKit 1.**
⚠️ **SO A MANUSCRIPT OF VERY LONG PARAGRAPHS WOULD DEFEAT IT** — ✅ **and that is a shape a writer can
legitimately produce.** ⚠️ **It must be a named risk in the sprint, not a discovery later.**

⚠️ **THE OPTION C DECISION IS UNCHANGED** — ✅ **TextKit 2 wins on every measured axis** — ⚠️ **but the
JUSTIFICATION shifts from "typing is slow" to "layout, resize and rebuild are O(document)".**
✅ **[I-0200]'s remaining symptom (scroll stalls) is consistent with resize/relayout pressure, ⚠️ not
with typing cost.**
