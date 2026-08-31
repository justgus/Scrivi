# Active Sprints

⚠️ **NO Sprint is currently active.**

✅ **SP-126 CLOSED 2026-08-30** → [`Closed/Sprint-SP-126.md`](Closed/Sprint-SP-126.md) — the three-tab
Scene Inspector shell, ⚠️ **closing EP-035 AC10** and retiring the "Scene Entities" placeholder.

✅ **SP-125 CLOSED 2026-08-29** → [`Closed/Sprint-SP-125.md`](Closed/Sprint-SP-125.md) — the first
Linux object surface.

✅ **SP-123 CLOSED 2026-08-29** → [`Closed/Sprint-SP-123.md`](Closed/Sprint-SP-123.md) — the rig is
BUILT, GREEN, and runs the app on real hardware.

### ⚠️ What is available to activate next

| Candidate | Epic | State |
| --------- | ---- | ----- |
| **SP-124** — ⚠️ **the DRIVE PULL** | EP-038 | 🔵 **Planned, now UNBLOCKED** — ⚠️ **EP-036's AC4 is UNSPECIFIABLE until T-0477 reports** |
| **AC3** — world binding + ⚠️ **the RELINK UI that does not exist** | EP-035 | ⚪ Not planned — ⚠️ **a moved world currently cannot be repaired from the app at all** |
| **AC4** — object CRUD | EP-035 | ⚪ Not planned |
| ⚠️ **Session persistence** (I-0176/0177/0178) | ⚠️ **NO EPIC** | ⚠️ **Apple's EP-018 equivalent — wants its own Epic** |
| **I-0180** — `[Apple]` object-card label | ⚠️ **NO HOME** | ⚠️ **EP-034 is closed** |

---
---

*Last Updated: 2026-08-30, second pass (**SP-126 ✅ CLOSED — user-approved**, archived to
[`Closed/Sprint-SP-126.md`](Closed/Sprint-SP-126.md) with its six Tasks verified and archived in the
SAME STEP → `Verified/Task-verified-0485-0490.md`. ✅ **The Audit Check was CLEAN**: 6/6 Tasks Verified,
0 unticked DoD items, nothing stale. ✅ **EP-035 AC10 is CLOSED.** ⚠️ **NO Sprint is now active** — see
the table above for what is available. ⚠️ **The largest unhomed item is session persistence**
(I-0176/0177/0178), Apple's EP-018 equivalent, ⚠️ **which wants its OWN Epic rather than being folded
into an existing one.** Next Task **T-0492**; ⚠️ **next Issue I-0181, opening a new decade.** Prior
note follows.)*

*Last Updated: 2026-08-30 (**SP-126's six Tasks ✅ ALL VERIFIED** — user-approved after a live pass on
the **REAL RIG** at build 8: *"all verified in app. no findings."* ✅ **The three-tab shell is
delivered**: Writing · Worldbuilding · Properties, ⚠️ **and "Scene Entities" — Apple's deleted SP-090
placeholder — is finally retired.** ✅ **571/571 ctest + 19/19 smokes on the rig**, Qt 6.10.2.
⚠️ **THREE defects were found by LOOKING, none by any suite**: the Properties tab unreachable at 200px
(scroll arrows, a gesture-only affordance); outline and todo NEVER SAVED (`QTextEdit` has no
`editingFinished`, and ⚠️ **my own comment claimed a focus-out hook I had not written**); and I-0179,
⚠️ **which took THREE rounds because the first two fixed the symptom** — the real defect was that the
relationship label was REDUNDANT on every row, ✅ **hoisted to the group header by user ruling.**
⚠️ **I-0180 filed against APPLE** — the same wrong label has shipped on macOS since EP-031 unnoticed;
✅ **building the surface a second time is what exposed it.** ⚠️ **SP-126 is NOT CLOSED** — that needs
explicit user approval. Next Task **T-0492**; ⚠️ **next Issue I-0181, opening a new decade.** Prior
note follows.)*

*Last Updated: 2026-08-29, third pass (**SP-123 ✅ CLOSED — user-approved**, archived to
[`Closed/Sprint-SP-123.md`](Closed/Sprint-SP-123.md) with its three Tasks verified and archived in the
SAME STEP → `Verified/Task-verified-0474-0476.md`. ✅ **The Audit Check before the close was CLEAN**:
7/7 DoD, all three Tasks Verified, nothing stale in backlog or unverified. ✅ **EP-038's AC1, AC2 and
AC3 are CLOSED.** ⚠️ **The rig runs the Linux app on REAL HARDWARE** — ⚠️ **and surfaced three Issues
(I-0176 no reopen, I-0177 no geometry, I-0178 single-project) within minutes, none findable by any
suite**; ✅ **user-ruled non-blocking**, and ⚠️ **I-0178 wants its OWN Epic** (Apple's EP-018
equivalent). ✅ **`docs/Scrivi_Linux_Rig_Setup_v0_1.md` written AS RUN** — ⚠️ **§7 (drive dismount)
deliberately EMPTY, owed by SP-124/T-0477.** ⚠️ **ONLY SP-126 is now active.** Next Task **T-0492**;
next Issue **I-0179**. Prior note follows.)*

*Last Updated: 2026-08-29, second pass (**SP-126 🟡 ACTIVATED** — EP-035's second sprint, ⚠️ **running
PARALLEL to SP-123**, which waits on `oathkeeper` being awake. ✅ **SP-126 is blocked on NOTHING** — the
four scene-notes endpoints are already bridged and `inspector-layout.json` needs no endpoint at all.
⚠️ **TWO CORRECTIONS FOUND AT PLANNING:** (1) ⚠️ **the draft's tab order was WRONG** — Apple's display
order is `Writing | Worldbuilding | Properties` (`InspectorTab` declaration order), defaulting to
Writing, ⚠️ **which is DISTINCT from the proving order** Writing → Properties → Worldbuilding;
(2) ✅ **the layout schema ALREADY EXISTS and is already populated** in the user's real projects —
`scrivi.inspector-layout.v1`, ⚠️ **APP-SIDE with no `scrivi_*` endpoint.** ⚠️ **T-0486 carries the
sprint's real risk**: Linux does not model `stackSort` or per-scene stacks, so it must PATCH the
document rather than reconstruct it — ⚠️ **reconstructing would silently delete the writer's
Apple-side card layout, invisibly until she reopened the project on the Mac.** ⚠️ **Accepted risk:
two app-side implementations of one schema; if a THIRD platform needs it, move it into ScriviCore.**
⚠️ **T-0491 (per-stack sort + drag-reorder) filed UNSCHEDULED**, user-ruled out of scope. Next Task
**T-0492**; next Issue **I-0176**. Prior note follows.)*

*Last Updated: 2026-08-29 (**SP-125 ✅ CLOSED — user-approved**, archived to
[`Closed/Sprint-SP-125.md`](Closed/Sprint-SP-125.md) with its five Tasks already verified and archived
2026-08-28. ✅ **The Audit Check before the close was CLEAN**: no Task in two layers, no stale status,
all three of its Issues settled. ⚠️ **AC10 (the three-tab shell) was NOT in SP-125's scope** — it did not
exist when the sprint was planned — ✅ **so the sprint closed on its stated scope rather than being
retroactively widened**; the shell is **SP-126**'s. ✅ **I-0171 VERIFIED and archived** in the same step.
⚠️ **Only SP-123 remains active, and it is BLOCKED ON THE USER.** Next Sprint **SP-126** (⚠️ **planning COMPLETE 2026-08-29**, six Tasks T-0485–T-0490, blocked on
nothing); ⚠️ **T-0491 filed UNSCHEDULED** (per-stack sort + drag-reorder, user-ruled out of SP-126);
next Task **T-0492**; next Issue **I-0176**.)*
