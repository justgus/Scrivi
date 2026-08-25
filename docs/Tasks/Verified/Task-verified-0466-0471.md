# Verified Tasks — T-0466 … T-0471

**Sprint:** [SP-122](../../Sprints/Closed/Sprint-SP-122.md) · **Epic:** EP-034 `[Cross]` Object Detail & Media
**Verified:** 2026-08-25 (user approval) · **Codebase:** `[Cross]` — ⚠️ **verification sprint; no feature work**

---

## The Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| **T-0466** | **L1 + L4** — `ctest` macOS arm64, interop tests, app build | ✅ **Verified** |
| **T-0467** | ⚠️ **L2 + L3** — **x86-64** + **sanitizer** runs — ⚠️ **the two legs that hide defects** | ✅ **Verified** |
| **T-0468** | **L5** — Linux container `ctest`, ⚠️ **tests ON, NON-ROOT, RE-RUN not cited** | ✅ **Verified** |
| **T-0469** | ⚠️ **THE LIVE-USE PASS on the real rig** — the primary instrument | ✅ **Verified** |
| **T-0470** | ⚠️ **Re-verify AC1–AC10 against their OWN triggers**, not re-cite | ✅ **Verified** |
| **T-0471** | **EP-034 close prep** — Audit Check + completion summary | ✅ **Verified** |

---

## ✅ AC12 — five legs, each RUN

| Leg | Result |
| --- | ------ |
| **L1** macOS arm64 | ✅ **567/567** (7.3 s) |
| **L2** ⚠️ **x86-64** | ✅ **567/567** (18.6 s under Rosetta) — ⚠️ **binary confirmed `Mach-O 64-bit x86_64`**, not a silently-native build |
| **L3** ⚠️ **sanitizers** | ✅ **567/567**, zero diagnostics — ⚠️ **confirmed LINKED: 55 `__asan`/`__ubsan` symbols vs 0 in the plain build** |
| **L4** interop + app | ✅ **127 tests / 12 suites**, ✅ **BUILD SUCCEEDED** |
| **L5** ⚠️ **Linux** | ✅ **571/571**, ⚠️ **NON-ROOT `uid=1001(scrivi)`**, second image built `SCRIVI_BUILD_TESTS=ON` |

⚠️ **L2 and L3 are the legs that had rarely been run.** `CMakeLists.txt:17-19` records why L2 matters:
**arm64 `sdiv` quietly yields 0 where x86-64 `idiv` raises `#DE`**, so "all tests pass" had meant
"all tests pass on arm64."

⚠️ **macOS 567 vs Linux 571 was CHECKED, not assumed** — the four extras are the Linux-only
`EncryptedFileSecureStore` tests (SP-059/T-0229).

### ⚠️ A toolchain error, caught by the user and DISCARDED rather than adjusted

⚠️ **The first L1 run used Xcode 26.6**, the `xcode-select` default. ⚠️ **The project requires Xcode 27**,
which lives at `/Applications/Xcode-beta.app` and ships **AppleClang 21** — a different compiler.
✅ **That result was thrown away, not reinterpreted**, and all five legs were re-run under a
session-scoped `DEVELOPER_DIR` rather than a machine-wide `xcode-select --switch`.

### ✅ I-0150 did NOT recur

⚠️ **The real rig was MOUNTED during `xcodebuild test`** (`/Volumes/Scrivi Worlds`, carrying
`Eskandar.scrivworld` and `the-stairs-of-tintagael.scrivi`). ✅ **Fingerprinted before and after: 625
files, identical mtime hash, nothing modified in the preceding 10 minutes.**

---

## ✅ T-0469 / T-0470 — the live pass, and a correction to this sprint's own premise

**The user ran the pass 2026-08-25 on the real rig. ✅ No failures across AC1–AC10.**

⚠️ **SP-122 was PLANNED expecting the pass to find defects. That expectation was WRONG**, and the user
ruled it so:

> *"We vetted all the features in the Apple App and you are having me do a regression test verbatim from
> previous tests. I would expect no failures at this point. And I didn't have any."*

⚠️ **The "22 consecutive Issues from clicking, none from a suite" statistic comes from passes over NEWLY
BUILT surfaces** — SP-118's related objects, SP-119's images, SP-120's sources. ⚠️ **A verbatim re-run
over already-vetted features is a REGRESSION test, and clean is the CORRECT result.** The statistic does
not transfer, and treating a clean pass as suspicious was a misreading of the project's own evidence.

✅ **What the pass DOES establish:** every AC1–AC10 behaviour still works after seven sprints of change,
on real data, with the Eskandar world attached over USB.

---

## ⚠️ Two Issues, NEITHER from the pass itself

| ID | Source | Outcome |
| -- | ------ | ------- |
| **I-0171** | ⚠️ **T-0468 — by RUNNING the Linux leg** | 🔴 **Open.** ⚠️ **SP-121's `.dockerignore` fix is INCOMPLETE**: five build dirs exist (`build/`, `build-tests/`, `build-linux-tests/`, `build-iphoneos/`, `build-iphonesimulator/`), both ignore files match only `build/`. ⚠️ **Filed, not fixed** — the fix needs a ruling |
| **I-0172** | ⚠️ **T-0469's console log** | ✅ **Verified + archived.** `ForkPopover` forced a nested AppKit layout pass; fixed via `sizeThatFits(in:)` |

⚠️ **The pass was clean; the findings came from running a build leg and from reading the log it produced.**

---

## ✅ T-0471 — the Audit Check earned its place

✅ **Seven mechanical checks run.** Clean on six.

⚠️ **The seventh found `Issue-Documentation.md` FOUR SPRINTS STALE** — it read *"2 Issues open"* and listed
**I-0140/I-0141 as open in SP-116**, ⚠️ **both fixed, Verified and archived on 2026-08-21** when that
sprint closed. ⚠️ **Without this check, EP-034 would have closed against an index claiming open Issues
that had been resolved four sprints earlier** — precisely the failure mode
`Audit-Guidelines.md` records for EP-031.

---

*Archived 2026-08-25 on user verification, in the same step the Tasks were marked Verified and SP-122 closed.*
