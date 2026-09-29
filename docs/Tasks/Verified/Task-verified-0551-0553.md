# Verified Tasks — T-0551 · T-0552 · T-0553

**Sprint:** ✅ **[SP-145]** → [`../../Sprints/Closed/Sprint-SP-145.md`](../../Sprints/Closed/Sprint-SP-145.md)
**Epic:** ✅ **[EP-043]** `[Linux]` **The Session** — ⚠️ **S1 of 4**
**Platform:** `[Linux]`
**Implemented:** 2026-09-27 · ✅ **USER-VERIFIED 2026-09-29 by live pass on the rig**
**Archived:** 2026-09-29, in the same step [SP-145] closed (`feedback_archive_on_close`).

---

## ✅ The verification

✅ **THE USER, 2026-09-29:** ***"the live test passed. hide inspector survived a restart."***

⚠️ **THAT ONE SENTENCE IS THE WHOLE POINT OF THE LIVE PASS.** ⛔ **`ctest` 641/641, 24/24 smokes and a
green boundary guard could NOT answer it** — ✅ **[I-0251] is a *"does it survive a quit?"* defect, and
`feedback_live_pass_finds_what_suites_cannot` says plainly that a green suite cannot.**

---

## The Tasks

| ID | Title | ✅ Status |
| -- | ----- | --------- |
| **T-0551** | ✅ **Introduce `ProjectSession`** — the seven per-project state members out of `EditorShell` (`projectID_`, `projectPath_`, `appSupportRoot_`, `bridge_`, `sceneDoc_`, `dirtyScenes_`, `activeSegment_`). ⚠️ **Behaviour-preserving** | ✅ **VERIFIED 2026-09-29** |
| **T-0552** | ✅ **Introduce `OpenProjectRegistry`** — `projectID` → live session ([EP-043] [R-Q2]) | ✅ **VERIFIED 2026-09-29** |
| **T-0553** | ✅ **[I-0251]** — pane visibility becomes per-project state; ⚠️ **inspector visibility persists THROUGH THE CORE** ([R-Q4]) | ✅ **VERIFIED 2026-09-29** — ⚠️ **the live-pass Task** |

⚠️ **ONLY T-0553 WAS OBSERVABLE.** ✅ **T-0551 and T-0552 are a refactor whose proof is mechanical:**
⛔ **`git diff --stat` on `tests/` showed 81 insertions and ZERO deletions** — ⚠️ **not one existing
assertion was altered, which is what [SP-145]'s behaviour-preserving mandate (AC2) required.**

---

## ⚠️ What the pass ALSO found — ⛔ none of it a regression here

| Finding | ⚠️ What it is |
| ------- | ------------- |
| ✅ **[I-0255]** | ⛔ **Hide TIMELINE did not survive a restart.** ⚠️ **AS DESIGNED at the time — [SP-145] AC5 says so explicitly.** ✅ **RULED 2026-09-29: it SHOULD persist** — ⛔ **so it is now a `[Cross]` defect on BOTH platforms** (⚠️ `ProjectSession.swift:98` is a plain `var` with no `didSet`) |
| ✅ **[I-0256]** | ⛔ **No control to hide the Scene Navigator on Linux.** ✅ **RULED 2026-09-29: ⛔ do NOT build a hide control — ⚠️ collapse the splitter to width 0.** ✅ **Cheaper than assumed: `EditorShell.cpp:160` applies `setCollapsible(false)` to the INSPECTOR only, ⚠️ so index 0 already collapses** |
| ✅ **[T-0556]** | ⚠️ **A feature request: `View ▸ Hide All` (focus mode) + `Restore All`, BOTH platforms.** ✅ **RULED to PERSIST across a restart** |
| ✅ **[T-0557]** | ⚠️ **Buffers palette visibility becomes per-session** — ⛔ **considered for this Sprint and correctly excluded** (✅ the palette is `#if os(macOS)`; ⛔ this Sprint is `[Linux]`) |

---

## ✅ What was RUN, not asserted

| Check | Result |
| ----- | ------ |
| `ctest` — ⚠️ **NON-ROOT (uid 1001), tests-on image** | ✅ **641/641, 0 failed** |
| Linux smokes (24 binaries, via their `.sh` wrappers) | ✅ **24/24 PASS** |
| `inspector_layout_smoke` | ✅ **30 checks, 0 failures** (⚠️ **13 → 21 assertion sites**) |
| `scripts/check-package-boundary.sh` | ✅ **GREEN — AC7** |
| Apple `xcodebuild -scheme ScriviApp build` | ✅ **BUILD SUCCEEDED** |
| ⚠️ **AC2 proven MECHANICALLY** | ✅ **81 insertions, ⛔ 0 deletions in `tests/`** |
| ✅ **THE LIVE PASS** | ✅ **PASSED 2026-09-29 — the inspector's hidden state survived a quit** |

### ⚠️ THE GUARD WAS PROVEN BY BREAKING IT

⛔ **A test that passes proves nothing until it can fail.** ✅ **The [I-0251] fix was temporarily
REMOVED and the suite rebuilt: `inspector_layout_smoke` went RED with exactly the two right failures**
— *"inspectorHidden was PERSISTED"* and *"a re-opened project reports the inspector as hidden"* —
✅ **then green again on restore.**

---

## ⚠️ The lesson worth keeping

⛔ **THREE OF [EP-043]'s FIVE RULING QUESTIONS WERE FRAMED ON A PREMISE THE CODE CONTRADICTED**, and in
each case the correction changed the ANSWER. ✅ **[I-0251] itself was found by READING APPLE'S SIDE
while ruling Q4** — ⛔ **not by a test.** ⚠️ **The question had been framed as *"should Linux reverse
SP-078?"*, which assumed a Linux-only decision; ✅ the code showed a PARITY GAP instead.**

⚠️ **AND THE LIVE PASS REPEATED THE PATTERN:** ✅ **it verified what it was for, ⛔ and found two more
gaps of the same shape** — ⚠️ **a capability that exists on one platform and not the other, or a
control whose state dies at quit.** ✅ **Both are now ruled.**
