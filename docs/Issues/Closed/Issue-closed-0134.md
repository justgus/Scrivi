# Closed Issue (Not Verified)

## I-0134: Apple and Linux disagree on what "restore where I was" means

**Status:** ⚪ **Closed — NON-ISSUE (2026-08-19, user-ruled)**
**Reason for Closure:** ⚠️ **Erroneous premise.** The Issue was filed as a cross-platform *parity
defect*, treating the Apple and Linux behaviours as two verified implementations in conflict, requiring a
ruling to arbitrate between them. **That premise is wrong.** Parity in Scrivi is **directional**:

> ⚠️ **Apple is the source of truth. Linux conforms to Apple.**

There was never a conflict to arbitrate — only **Linux work not yet scheduled.** A difference between the
two platforms is **the expected state** during development, not a defect, and must not be filed as one.

**User ruling (2026-08-19), verbatim in substance:**
> *"I-0134 should never have been written to start with. It involves parity with Linux, which is an
> erroneous requirement. Linux is required to have parity with Apple, so we're first working on the
> 'source of truth' for the app and will make changes to Linux later."*

**Re-open condition:** ⚠️ **None.** This Issue does not become valid later. If Linux's restore behaviour
needs to change, that is **scheduled Linux work** (EP-026 or a successor), raised as a Task on its own
terms — **not** re-opened as a cross-platform Issue.

**Platform:** macOS + Linux
**Component:** `ViewportSceneLoader.swift` (Apple), `EditorShell.cpp:364` (Linux)
**Severity:** Low
**Sprint:** Was unassigned (listed as an "EP-026 candidate") at closure
**Date Identified:** 2026-08-18
**Date Closed:** 2026-08-19
**Closed under:** Audit ruling **R-01** — [`../../Audits/Audit-Rulings-20260819.md`](../../Audits/Audit-Rulings-20260819.md)
**Related:** [`I-0133`](../Verified/Issue-verified-0131-0140.md) (the Issue whose investigation produced
this one), I-0131, T-0247 (EP-022)

---

**What was observed (the observation was accurate; the conclusion was not):**

**Apple** centres the restored scene and ignores `restored.scroll` entirely — ruled in I-0131 and §1 of
the Current Scene Model. The reason is specific: a scene parked at the viewport edge makes the scroll
handler read a *different* scene at the centre, and the navigator highlight then follows that wrong scene.

**Linux** does the opposite. `EditorShell.cpp:364` calls `centerCursor()` and then **overrides it** with
the saved fraction, commented: *"Apply the saved scroll fraction over the whole document (overrides the
centerCursor scroll when a real fraction was persisted)."*

Both behaviours were verified on their own platform — Linux's as **T-0247**, VNC-verified 2026-07-15 and
closed with EP-022; Apple's as **I-0131**, verified 2026-08-18.

---

**Why this was closed rather than resolved:**

The original entry concluded: *"Neither is a defect in isolation; the defect is that the same project
reopens differently depending on which app you use."* ⚠️ **That reasoning treats the two platforms as
peers.** They are not. Apple is where behaviour is decided; Linux follows once the Apple behaviour is
settled. Filing the gap between them as a defect creates work that is really just *"Linux has not caught
up yet"* — which is true of many behaviours at any given moment and is not a bug list.

⚠️ **Nothing is owed by this closure.** No Apple change, no Linux change, no EP-026 item.

---

**⚠️ Do not revive Apple's scroll fraction.** [[I-0133]] documents why the Apple side was removed
deliberately, and I-0131 documents why edge-restore is wrong there. If parity is ever resolved toward
centring, **Linux's override is what changes** — and that means re-verifying a shipped, user-verified
EP-022 behaviour over Docker+VNC.

---

*Closed 2026-08-19 under audit ruling R-01. The observation is preserved for reference; the framing is
explicitly repudiated.*
