# Verified Task: T-0541

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0541** | ⚠️ **The [I-0197] regression guard — BUILD it and WIRE it into CI** | [SP-149] | [EP-041] | **2026-09-22** |

✅ **VERIFIED by a GREEN GitHub Actions run: "Scrivi Apple CI #1", commit `a25e106`, 2026-09-21 18:19 EDT.**

---

## What it built

- ✅ **`scripts/check-package-boundary.sh`** — ⚠️ **ONE script, TWO pattern sets, BOTH platforms.**
- ✅ **`.github/workflows/scrivi-apple-ci.yml`** — ⚠️ **`Scrivi/`'s FIRST CI OF ANY KIND.**
  ⛔ **Lint-only (no `xcodebuild`), on `ubuntu-latest`: greps do not need a Mac.**
- ✅ **`check-textkit2.sh` WIRED IN BESIDE IT** — ⚠️ **written and verified-failing in SP-133, then run
  by NOTHING until this Task.**
- ✅ **The Linux half in `scrivi-linux-ci.yml`, running FIRST** — ⚠️ **before the ~3-min Qt install,
  because a boundary violation is knowable from source alone.**

## ⚠️ The allow-list mechanism changed mid-Task, ON EVIDENCE

⛔ **Line numbers were tried first and were wrong WITHIN MINUTES:** ⚠️ **the very next edit — a
four-line tombstone — shifted two legitimate sites and the guard went red on them.**
✅ **Replaced with a trailing `// boundary-ok: <reason>` marker AT the site.** ⚠️ **A line number in
the script is a second copy of a fact the source already knows (P7);** ✅ **a marker moves WITH the
code and is visible to the next reader.**
✅ **A marker with NO reason after the colon FAILS THE GUARD** — ⚠️ **that is what stops the allow-list
becoming the dumping ground [SP-143]'s plan names as its first risk.**

⛔ **WHOLE-FILE ALLOW-LISTING WAS ALSO REJECTED, and tested.** ⚠️ **`TimelineStripView.swift` is where
[SP-129] had to remove a bypass, and `EditorShell.cpp` is where [I-0241]'s disk walk lived** —
✅ **exempting either wholesale would blind the guard in the two files that have already earned this
defect once.**

## How it was verified

⚠️ **LOCALLY — VERIFIED FAILING FOUR WAYS** (⛔ **a green test that cannot fail is not evidence**):
1. ✅ **Apple bypass** (`Data(contentsOf:)`) → exit 1, line named.
2. ✅ **Linux bypass** (`QFile`) → exit 1, with the [I-0241]-specific advice.
3. ✅ **A bypass INSIDE an allow-listed file** → exit 1 — ⚠️ **the blind spot closed.**
4. ✅ **A `boundary-ok:` marker with no reason** → exit 1.

✅ **Both guards also pass in a BARE `ubuntu:24.04` container with no toolchain** — ⚠️ **which is what
proves the lint-only job works on the runner it is configured for.**

✅ **ON GITHUB — THE EVIDENCE ONLY A PUSH COULD PROVIDE:** ⚠️ **YAML validity and working scripts do
NOT prove a workflow RUNS** — ⛔ **a path-filter typo, a bad `runs-on` or a missing checkout all pass
local checks and fail silently.** ✅ **`a25e106` touched `Scrivi/**`, the Apple workflow TRIGGERED, and
run #1 went GREEN.**

## ⚠️ What this Task's verification does NOT cover

⛔ **A green run proves the guard RUNS and PASSES on clean code.** ⚠️ **It does NOT prove CI goes RED
on a violation** — ✅ **that was proven locally four ways, ⛔ never through GitHub.**
✅ **That is [SP-143]'s D1, deliberately: a guard written when the code was ALREADY CLEAN has only ever
seen green, so it needs an independent witness against the defect it exists for.**

---

*Verified 2026-09-22 by a green CI run. Implemented under [SP-149] / [EP-041].*
