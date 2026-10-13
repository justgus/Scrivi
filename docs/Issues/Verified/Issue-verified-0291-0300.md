# Verified Issues: I-0291 – I-0300

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0281–I-0290 are in [`Issue-verified-0281-0290.md`](Issue-verified-0281-0290.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0291** | `[Apple]` ⚠️ **The Buffers palette's button switched to PASTE on ⌃ alone after paste moved to ⌃⌘ ([I-0290]).** Reported by the user 2026-10-12. ✅ Fixed: it follows ⌃⌘, as the chord does. → [details](#i-0291) | **Low** | [SP-179] | ✅ **VERIFIED 2026-10-12** |
| **I-0292** | `[Build]` ⛔ **Scrivi Linux App CI red on every push since SP-176: `find_smoke`'s timing check (< 5000 ms) ran against an UNOPTIMIZED build (6.7–6.8 s).** The workflow configured no build type (GCC -O0); the rig and Docker build Release. ✅ Fixed: CI builds Release. → [details](#i-0292) | **Medium** | — (unassigned) | ✅ **VERIFIED 2026-10-13** |

---

## I-0291

**Title:** The Buffers palette's button switches to paste on ⌃ alone, not ⌃⌘
**Status:** ✅ **VERIFIED 2026-10-12** (user, Mac: *"I-0291 passes"* — the four steps below, including the ⌃⌘-click paste).
**Platform:** `[Apple]`
**Component:** `BuffersPalette.swift` `BufferAction.current`
**Severity:** Low — the palette's paste was reachable, just on a modifier that no longer matches the chord
**Sprint:** [SP-179]
**Date Identified:** 2026-10-12 — user, on the Mac: *"I-0290 passes. But the view buffers icon changes on holding Ctrl down, not
Ctrl-Cmd."*

**Root cause:** my choice in [I-0290] — I kept the palette on ⌃ alone (*"a click, not a chord"*) and said so; it does not match
the chord the hint names.

**Fix:** `BufferAction.current` returns paste only for ⌃ AND ⌘; ⌃ alone → copy. Hint: *"hold ⌃⌘ or ⌥ to change the button"*.
`BufferAction` made internal (was private) so a test can reach it.

**Tests:** `BufferPasteChordTests.paletteButtonFollowsTheChord` — FAILED on the old code (⌃ alone → paste), passes now. Interop
284/284. ✅ A ⌃⌘-CLICK on the button fires it — checked by the user on the Mac (verify step 4); no test covers the click.

**To verify (Mac):** open View ▸ Show Buffers. Hold ⌃ → the icons stay Copy. Hold ⌃⌘ → Paste icon; click a filled slot's button
with ⌃⌘ held → it pastes at the caret.

---

## I-0292

**Title:** Scrivi Linux App CI fails on every push since SP-176 — a timing check measured against an -O0 build
**Status:** ✅ **VERIFIED 2026-10-13** (user: *"ci-linux passes now. mark I-0292 verified"* — run 38315453907, commit `23237d2`, green)
**Platform:** `[Build]` — `.github/workflows/scrivi-linux-ci.yml`
**Severity:** Medium — the Linux CI signal has been red, so a real Linux failure would not stand out
**Sprint:** — unassigned (user: *"Scrivi's ci-linux is failing again."*)
**Date Identified:** 2026-10-12 (failing since 2026-10-11, run 38167539536, the SP-176 live-pass push)

**Evidence:** four consecutive runs (38167539536, 38174077102, 38267461495, 38310102628) fail ONLY on `find_smoke`:
*"FAIL: the build stays in seconds, not minutes (6703 / 6737 / 6826 / 6774 ms)"* — `tests/find_smoke.cpp:315`, `buildMs < 5000`
on a 1,186-scene, 1.86 MB manuscript. ⚠️ [SP-176] ticked *"`find_smoke` in CI"*; the push that closed it was the first red run.

**Root cause:** the workflow's Configure step set no `CMAKE_BUILD_TYPE`, so GCC built at -O0. Every other Linux build is Release
(the rig, `Scrivi_Linux_Rig_Setup_v0_1.md:231`; the Docker image, `Dockerfile:51`). Measured 2026-10-12 in the Docker image:
the same smoke builds the presented text in **284 ms (Release)** vs **1179 ms (CI's configuration)** — 4.2×. The rest of CI's
6.7 s is the hosted runner. ⚠️ Inferred, not measured: Release on the runner ≈ 6.7 / 4.2 ≈ 1.6 s.

**Fix:** `-DCMAKE_BUILD_TYPE=Release` in the workflow's Configure step, with the reason. ⛔ The threshold is NOT loosened — it
exists to catch a quadratic build, and it must measure the build that ships.

---

*Last Updated: 2026-10-13 (**I-0292** VERIFIED (user).)*

*Last Updated: 2026-10-12 (batch created: **I-0291** VERIFIED (user).)*
