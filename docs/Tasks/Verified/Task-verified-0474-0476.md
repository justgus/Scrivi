# Verified Tasks — T-0474 – T-0476 (SP-123, EP-038)

**Sprint:** SP-123 — `[Linux]` Rig reachable + building natively (✅ **CLOSED 2026-08-29**)
**Epic:** [EP-038](../../Epics/Epic-active.md) — `[Linux]` The Real Hardware Rig · **sprint 1 of 2**
**Verified:** 2026-08-29 — **user-approved**
**Codebase:** ⚠️ **environment/infrastructure — no application code changed**

---

## ⚠️ Ownership was SPLIT, and T-0474's verification direction was INVERTED

| ID | Title | Implemented by | Verified by |
| -- | ----- | -------------- | ----------- |
| **T-0474** | Account + SSH key exchange + reachability | ⚠️ **THE USER** (2026-08-27) | ⚠️ **CLAUDE** |
| **T-0475** | Toolchain + dependencies | **Claude**, over SSH | The user |
| **T-0476** | First native build + `ctest` + ⚠️ **app on a REAL display** | ⚠️ **THE USER** | The user |

⚠️ **The standing rule "Claude may never mark a Task Verified" did NOT bar T-0474** — ✅ **it exists to
stop Claude self-certifying its OWN work**, and T-0474 was the user's. ⚠️ **This is the only Task in
the project so far with that direction.**

---

## ✅ Evidence

- ✅ **Native build OUTSIDE a container**, from a clean tree at `~/Dev/Scrivi`
- ✅ **`ctest` 571/571 NON-ROOT, tests ON** — ⚠️ **compared against 571, not skimmed**
- ✅ **18/18 Linux smokes**
- ✅ **The app LAUNCHED on a real session over RDP** and opened a project —
  ⚠️ **navigator, manuscript view and timeline all nominal.** ⚠️ **Xvfb would not have satisfied this.**

### ⚠️ The rig is far ahead of the container, and it cost nothing

| | Container (24.04) | **Oathkeeper (26.04.1)** |
| - | ----------------- | ------------------------ |
| CMake | 3.28.3 | **4.2.3** |
| GCC | 13.3.0 | **15.2.0** |
| Qt | 6.4.2 | **6.10.2** |

⚠️ **Six Qt minor versions and two GCC generations apart, ZERO code changes.** ⚠️ **The CMake 4 risk
was real and did not fire** — nlohmann/json 3.11.3's `VERSION 3.1...3.14` range saved it; ⚠️ **a future
dependency without a range will fail here.**

---

## ⚠️ Issues raised — three, within minutes of the first launch

| ID | Finding |
| -- | ------- |
| **I-0176** | A project open at Quit does not reopen |
| **I-0177** | A maximized window does not reopen maximized — ⚠️ **splitter sizes are not persisted either** |
| **I-0178** | ⚠️ **Only ONE project can be open at a time** |

⚠️ **ONE gap with three symptoms.** ✅ **Apple solved all three in EP-018** (R1–R5). ⚠️ **I-0178 is the
parent and wants its OWN Epic** — a structural rework of `ScriviWindow`/`EditorShell`.
✅ **User-ruled non-blocking**: gaps in scope never claimed, not failures of what was built.

⚠️ **None was findable by a suite** — they are about what survives a QUIT.

---

## ✅ The deliverable that survives

**`docs/Scrivi_Linux_Rig_Setup_v0_1.md`** — written **AS RUN**. ⚠️ **The Windows rig must EXECUTE it,
not re-derive it.** ⚠️ **Its §7 (drive dismount) is deliberately EMPTY** — SP-124 / T-0477.

---

*Verified 2026-08-29 by user approval. Archived in the same step SP-123 closed
(`feedback_archive_on_close`).*
