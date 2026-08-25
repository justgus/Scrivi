# Verified Tasks — T-0460 … T-0465

**Sprint:** [SP-121](../../Sprints/Closed/Sprint-SP-121.md) · **Epic:** EP-034 `[Cross]` Object Detail & Media
**Verified:** 2026-08-25 (user approval) · **Codebase:** `[Linux]` — ⚠️ **`ScriviBridge` only; no UI shipped**

---

## The Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| **T-0460** | ⚠️ **The gap audit, MECHANICAL** — all **100** endpoints, each present / missing / deliberately N-A with a reason | ✅ **Verified** |
| **T-0461** | **Objects + kinds + worlds** — ⚠️ **`list_object_kinds` is load-bearing**; kind scope DERIVED, never restated | ✅ **Verified** |
| **T-0462** | **Edges + relation types + assets** — ⚠️ **edge labels passed through, never recomputed** | ✅ **Verified** |
| **T-0463** | **Scene metadata, comments, inbox, repair, snapshots** — the long tail | ✅ **Verified** |
| **T-0464** | ⚠️ **Bridge tests — every endpoint through `ScriviBridge`, NOT the C ABI** (I-0113's lesson) | ✅ **Verified** |
| **T-0465** | ⚠️ **THE PORTING OUTLINE** — written for **FIVE** platforms, not one | ✅ **Verified** |

---

## What was delivered

**`ScriviBridge` went from 34 to 81 of 100 endpoints** — ⚠️ **verified by re-running T-0460's audit, not
asserted.** The only 19 outstanding are the `history` (15) + `buffers` (4) set, ⚠️ **deliberately excluded
and owned by EP-019**, whose Linux story has never been ruled.

| Evidence | Result |
| -------- | ------ |
| Endpoints bridged | ✅ **34 → 81 of 100** |
| `ctest` **on Linux**, non-root, tests **ON** | ✅ **571 cases / 9,439 assertions, ALL PASSED** — ⚠️ **the leg SP-116 could never run** |
| `ctest` macOS | ✅ **567/567** unchanged (no ScriviCore code touched) |
| Bridge parity smoke (T-0464) | ✅ **28 checks, 0 failures**, non-root |
| All Linux smoke tests | ✅ **17/17**, no regressions |
| Linux container build | ✅ **BUILD SUCCEEDED**, zero compiler diagnostics |

⚠️ **macOS 567 vs Linux 571 was CHECKED, not assumed:** the four extras are the Linux-only
`EncryptedFileSecureStore` tests (SP-059/T-0229); Apple's `KeychainSecureStore` was deleted.

---

## ⚠️ Two corrections this sprint made to its own plan

1. ⚠️ **The endpoint count was 100, not 102.** Planning grepped prose and `scrivi_free` as if they were
   declarations. ✅ **This is precisely why T-0460 landed FIRST** — a wrong denominator would have surfaced
   at close as "scope creep."
2. ⚠️ **In-scope work was 47, not 43** — a 9% under-estimate, found by the audit rather than at close.

⚠️ **A third correction was found later, during the 2026-08-25 Audit Check:** the plan said **two**
endpoints were unreached on Apple. ⚠️ **The measured number is FOUR** — `resolve_timeline_project_times`,
`set_timeline_epoch_offset`, `set_world_epoch_offset`, `upsert_relation_type`. Apple reaches **96 of 100**.
⚠️ **`upsert_relation_type` needs an EP-035 consumer**; the three timeline endpoints are unclaimed.

---

## A latent defect fixed on the way

⚠️ **The repo had NO `.dockerignore` while the Dockerfile does `COPY . /src`.** A local macOS `build/`
therefore entered the image with a host-path `CMakeCache.txt` — ⚠️ **succeeding under `--no-cache` and
FAILING on a cached rebuild**, with an error naming CMake rather than the cause. ✅ **Fixed.**
⚠️ **Pre-existing, not introduced here** — and plausibly why the Linux leg had been skipped before.

---

## ⚠️ The honest caveat, recorded at verification

⚠️ **These six Tasks were verified by SUITE, not by USE** — user-ruled (SP-121 §3), because no
writer-facing surface shipped. ⚠️ **That evidence class has never caught this project's real defects:
22 consecutive Issues across SP-118–SP-120 came from clicking, none from a suite.**

⚠️ **The click-through is DEFERRED to EP-035, not waived.** Until EP-035 runs, all 47 bridged endpoints
are exercised only by tests written alongside them.

---

*Archived 2026-08-25 on user verification, in the same step the Tasks were marked Verified and SP-121 closed.*
