# Verified Issues — I-0181 … I-0190

⚠️ **New decade file.** The previous decade closed at **I-0180**
([`Issue-verified-0171-0180.md`](Issue-verified-0171-0180.md)).

⚠️ **All FOUR opening entries came from ONE live pass** (T-0496, SP-127) and ⚠️ **all
were found by the USER** — two by looking at a real screen, one by doing the thing the
sprint's own risk table said to test, one by using the app the way a writer would.
⚠️ **None was caught by a suite.** ⚠️ **Two of the four (I-0184, I-0185) needed the user
to CORRECT MY DIAGNOSIS before the real defect came into view.**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0183** | `[ScriviCore]` ⚠️ **DATA LOSS — relinking a world to a COPY of itself is ACCEPTED, and silently DROPS the project's relationships.** ⚠️ **Found by the USER on the REAL RIG during the SP-127 live pass (T-0496 step 7), 2026-09-01.** ⚠️ **`the-lone-golem`'s relationship log went from 12 live edges to 2** at 17:37:59 — ⚠️ **Myton→Ch1-Sc1 and Petch among those destroyed** (`objects/relationships.jsonl`, 3620 B → 491 B). **Root cause:** `WorldStore::relink` (`ScriviCore/src/worlds/WorldStore.cpp`) DOES verify identity — ⚠️ **but it compares `worldID`, and a COPY carries the SAME `worldID`.** ⚠️ **Its own comment says it exists so *"a same-named package would otherwise be substituted"* — yet a copy is EXACTLY the case it cannot detect.** The relink succeeded, then `refreshCachedIndex` reconciled the project against the copy and the edges were dropped. ✅ **The objects were NOT lost** — ⚠️ **an intermediate claim in this session that the bound package was EMPTY was WRONG** (I checked `objects/character/`; the layout is top-level `characters/`) — ✅ **the package holds all 35 objects, Myton and Petch included, with MATCHING objectIDs.** ⚠️ **So this is a RECONCILE defect, not world corruption**, which makes it worse: the world was fine and the project was damaged anyway. ⚠️ **`worldID` equality is the WRONG QUESTION** — ✅ **two packages sharing a `worldID` are the SAME world at DIFFERENT REVISIONS, and reconciling against the older one is a silent rollback.** ⚠️ **A relink must not DELETE edges it cannot confirm** — ✅ **an edge whose object is absent from the newly-bound package should be held PENDING (the machinery already exists — that is what unlinked/pending objects ARE), never removed.** ⚠️ **Compare `project_unlinked_world_objects_are_normal`: unresolved links are NORMAL and must never be swept.** ✅ **User's data recoverable** from `~/ScriviLinux/projects/the-lone-golem.scrivi/` (same `projectID`, 14 edges + 2 tombstones, contains the Myton edge; both surviving edgeIDs present, so the restore is a strict superset). ✅ **ROOT CAUSE FOUND 2026-09-01, and it is NOT the `worldID` check.** ⚠️ **`repairDangling` runs on PROJECT OPEN, not on relink** (`ProjectOpener.cpp:81`) — ⚠️ **so the relink itself destroyed nothing; the RELOAD that followed did.** ⚠️ **The prune logic was already CORRECT** (*"PENDING WINS OVER DANGLING, unconditionally… Absence is never deletion"*). ⚠️ **The hole was UPSTREAM in `EndpointResolver`:** the world resolved **`available`** — because `WorldStore::resolve` reads `world.json`, a **SMALL** file still served from the client cache — ⚠️ **while `index.json` (larger) failed `EBADF` on the phantom mount.** ⚠️ **`loadWorldIndex` then fell back to `scanDir`, which swallows EVERY I/O failure with `continue`** — ✅ **correct for building an index, one bad file must not cost the rest** — ⚠️ **but it makes an UNREADABLE tree and an EMPTY one produce IDENTICAL results.** ⚠️ **The `available` branch then `continue`d WITHOUT consulting the cached index**, so the endpoint came back `found=false, worldPending=false` = **`dangling`** = licensed to prune. ✅ **FIX (three parts):** (1) `scanDir` reports `sawIOError`; ⚠️ **a failed `exists()` is "I could not tell", NOT "no such directory"**; (2) `loadWorldIndex` reports `indeterminate` ⚠️ **and REFUSES to persist a scan it could not trust** — ⚠️ **writing that empty index into the package would have made the loss PERMANENT, outliving the mount that caused it**; (3) `ResolvedEndpoint::worldIndeterminate` — ⚠️ **an available-but-unreadable world now falls through to the cached index and yields `pending`, never `dangling`.** ✅ **REGRESSION TEST PROVEN TO CATCH IT** (`RelationshipTests.cpp`, `[I-0183]`): ⚠️ **verified FAILING against the unfixed core** — edge pruned, 0 held, graph drops to 0 — ✅ **and passing with the fix.** ✅ **`ctest` 568/568 macOS.** ⚠️ **The `worldID`-equality weakness is REAL but SEPARATE and LESSER** — ⚠️ **a copy shares `worldID` AND `modifiedAt`, so no field currently distinguishes revisions; ✅ with this fix a wrongly-bound world can no longer DESTROY anything, only leave links pending.** ✅ **User's data RESTORED 2026-09-01** — Ch1-Sc1's three relationships (Tintagael, Pet, Myton at 23) verified back; damaged file kept as `relationships.jsonl.damaged-20260901-180016`. ✅ **REPRODUCED AND FIXED ON THE RIG 2026-09-01 (build 9).** ⚠️ **Verified against a CONTROL, not just asserted:** the same real project data (12 live edges) and the same available-but-unreadable world were opened through `scrivi_open_project` — the exact ABI path that ran the prune — against **two cores built from the same tree, differing only in this fix**. ⚠️ **UNFIXED: 12 live → 0 live**, log grew 3620 → 4664 B as tombstones were appended — ⚠️ **the data loss reproduced exactly.** ✅ **FIXED: 12 live → 12 live**, log **byte-identical at 3620 B.** ✅ **`ctest` 572/572 + 19/19 smokes on Linux**, and `[I-0183]` passes on the rig. ✅ **Run on DISPOSABLE COPIES** — ⚠️ **the user's real project was never opened by the probe** and is untouched (3620 B). ✅ **VERIFIED FROM THE WRITER'S SEAT 2026-09-02.** ⚠️ **The user reproduced the ORIGINAL trigger by hand** — renamed the package so the bound path was unreadable, reloaded — ✅ **and the Scene Inspector said *"Objects held pending"*, NOT *"missing"*.** ⚠️ **That is the exact state that destroyed 10 of 12 relationships before the fix.** ✅ **Relinking to the correct package CLEARED the pending notices without a scene change**, and ✅ **the data was verified intact afterwards: `the-lone-golem` 12 live edges, `the-stairs-of-tintagael` 46 — nothing pruned across the rename, the pending state, or the relink.** | ⚠️ **HIGH** | ⚠️ **Unassigned** — ⚠️ **found by T-0496 live pass** | ✅ **VERIFIED 2026-09-02 (user-approved)** |
| **I-0184** | `[Linux]` **The Worlds dialog's package path is not readable — it elides against a FIXED width, not the row's actual width.** ⚠️ **Found by the USER during the SP-127 live pass (T-0496 step 2), 2026-09-01** — reported as *"only shows the world name and a label if the world is not available"*, i.e. ⚠️ **the path the DoD requires is effectively invisible.** ✅ **The row DOES build a path label** (`WorldsDialog.cpp:181`), and ✅ **the envelope key is correct** — ⚠️ **an intermediate claim in this session that the core never emits `lastKnownPackagePath` was WRONG**; it is emitted at `scrivi_c_api.cpp:1181` and parsed at `WorldsDialog.cpp:139`. **Root cause is the ELIDE:** `makeElidedPathLabel` calls `fm.elidedText(path, Qt::ElideMiddle, 360)` — ⚠️ **a HARDCODED 360 px that ignores the label's real width**, so on a narrower dialog (or a larger RDP font) the path collapses to little more than an ellipsis. ⚠️ **It also never re-elides on resize**, so widening the dialog does not reveal more. ✅ **Fix: elide against the label's own width in a `resizeEvent`/`eventFilter`, not a constant.** ✅ **The full path IS reachable as a tooltip**, which is why this reads as "no path" rather than "wrong path". ⚠️ **DoD requires "name, path, status"** — ⚠️ **so this blocks that checkbox.** ✅ **FIXED 2026-09-01 (build 14).** ⚠️ **THE FIRST DIAGNOSIS WAS WRONG and is corrected here:** the hardcoded 360 px was ⚠️ **never what truncated the path.** ✅ **MEASURED on the rig:** real world paths are only **230–278 px**, comfortably under 360 — ⚠️ **so the constant never engaged.** ⚠️ **The defect is the ROW LAYOUT:** in a 560 px dialog the row is 556 px, the status label takes **147 px** and the buttons their natural widths, leaving the path label **202 px** for a path needing **211 px**. ⚠️ **Eliding against 360 px when the label holds 202 px means the text is NOT ELIDED AT ALL — so Qt CLIPS it, and clipping removes the TAIL**, which is the package name, the one part that identifies the world. ✅ **Confirmed by direct measurement of the pre-fix widget: 211 px of text in a 202 px label → CLIPPED.** ✅ **FIX, two-sided:** (1) elide against the label's **own** `contentsRect().width()`, ⚠️ **at PAINT time as well as on resize** — ⚠️ **`resizeEvent` alone is insufficient: a widget laid out but never shown gets no resize event, so the FIRST paint would still be unelided** (⚠️ **this gap was in my own first fix and the smoke caught it**); (2) `QSizePolicy::Fixed` on the marker and status label, `Ignored` on the name, so the fixed-width siblings stop taking the path column's slack. ✅ **New smoke `worlds_path_smoke`** asserts at the **measured 202 px**: the tail survives, the rendered text is not wider than the label, widening reveals more, and a wide row shows the path in full. ⚠️ **The smoke ALSO had to be corrected twice** — it omitted the widget's 1 pt font shrink, and it called `resize()` without a paint. ✅ **`ctest` 572/572 + 20/20 smokes on the rig.** | Low | ⚠️ **Unassigned** — found by T-0496 live pass | ✅ **VERIFIED 2026-09-02 (user-approved)** — seen in the running app on build 17 |
| **I-0185** | `[Linux]` **A package is a DIRECTORY, so the file dialog DESCENDS INTO it instead of treating it as an atom.** ⚠️ **Found by the USER (T-0496 step 6) 2026-09-01, and CORRECTED BY THEM TWICE.** ⚠️ **Correction 1 (09-02):** it is not a "picker" but ⚠️ **the standard file dialog, at THREE sites** — including ⚠️ **`Open Project`, the app's most common action.** ⚠️ **Correction 2 (09-02), which invalidated my first fix:** ⚠️ **single-click + "Choose" ALWAYS worked and was never the complaint.** ⚠️ **The complaint is the DOUBLE-CLICK: the dialog descends and sits inside the package showing `manuscript/`, `objects/`, the `.json` files.** ✅ **The user's rule:** *"The writer must perceive the folder structure as if it were a single file… in the app it must appear as if it were an atomic thing."* ⚠️ **MY FIRST FIX WAS WRONG AND IS RECORDED AS SUCH:** it let the dialog descend and then resolved the RETURN VALUE back up to the package — ⚠️ **which fixes the path while leaving the writer standing inside the package looking at its internals, i.e. it did not address the complaint at all.** ⚠️ **It also passed every pure-function test I wrote for it**, which is why the smoke now drives a REAL double-click. ✅ **FIX (build 29):** `PackageFolderDialog.hpp` — a **non-native** `QFileDialog` with an **event filter on both item viewports** that ⚠️ **SWALLOWS the double-click on a package and CHOOSES it instead**, so ⚠️ **navigation never happens.** ⚠️ **Three Qt obstacles, each recorded in the header:** `accept()`/`done()` are **protected** (hence a small `ChooserDialog` subclass); ⚠️ **`selectFile()` does NOT work in Directory mode** — `selectedFiles()` reports the VIEWED directory, so a double-clicked package came back as its PARENT (hence an explicit `chosen` member); and the filter must sit on the **viewport**, not the view. ✅ **Accept button relabelled "Choose".** ⚠️ **`packageRootFor` kept as a BACKSTOP** for a typed path. ⚠️ **NOT applied to "choose where to CREATE a project"** — descending is correct there. ⚠️ **INVISIBLE ON APPLE** (Cocoa bundles) — ⚠️ **every Qt platform re-earns it; Windows next.** ✅ **`package_folder_smoke` drives a REAL `QTest::mouseDClick`** and is ⚠️ **verified to FAIL without the filter (times out — the dialog descends and never returns).** ✅ **572/572 + 22/22.** | Low | ⚠️ **SP-127** — ⚠️ **user ruled 2026-09-02 to keep the sprint ACTIVE and fix it here** | ✅ **VERIFIED 2026-09-02 (user-approved)** — *"It works perfectly."* |
| **I-0186** | `[Linux]` ⚠️ **Secondary text is INVISIBLE — `palette(mid)` is a STRUCTURAL role with no contrast guarantee, and it was used as a TEXT colour.** ⚠️ **Found by the USER on the REAL RIG, 2026-09-02:** *"my environment is set to dark mode and the text is not being displayed as white in dark mode."* ✅ **MEASURED on the rig's actual themes:** ⚠️ **Yaru-dark → `Mid` `#2f2f2f` on `Window` `#2a2a2a` = 1.07:1**; ⚠️ **Yaru light → `Mid` `#ffffff` on `#fcfcfc` = 1.03:1.** ⚠️ **WCAG AA needs 4.5:1 — so the world PATH was invisible in BOTH real themes, not just dark.** ⚠️ **`palette(link-visited)`, used as the "warning" colour, resolved to MAGENTA `#ff00ff`** — an unreviewed colour that merely happened to be visible. ⚠️ **THE REASON IT SURVIVED EVERY CHECK, and the lesson worth keeping: Qt's NO-THEME fallback renders `Mid` as a pleasant `#b8b8b8` grey.** ⚠️ **Offscreen smokes and headless screenshots use that fallback** — ⚠️ **so the whole suite AND my own screenshot of the row showed a perfectly readable path that no real user ever saw.** ⚠️ **A HEADLESS RENDER IS NOT EVIDENCE ABOUT COLOUR.** ✅ **FIX:** new shared `platforms/linux/src/ThemeColours.hpp` — ⚠️ **derive secondary text by blending `WindowText` toward `Window`** (the two colours a theme guarantees are legible against each other) ⚠️ **rather than naming any role**; a theme-polarity-aware amber for "needs attention"; applied via the **palette**, not a stylesheet (⚠️ **a stylesheet colour overrides the theme permanently and ignores a runtime light/dark switch**), on **both** the Active and Inactive groups. ⚠️ **Also removed `SceneInspector`'s `setEnabled(false)`-as-dimming**, which routed the label through the DISABLED palette group and overrode the colour — the same class of bug. ✅ **Affected BOTH Linux surfaces** (`WorldsDialog`, `SceneInspector`) — ⚠️ **hoisted, not restated, per the `WorldStatusText` precedent.** ✅ **New smoke `theme_contrast_smoke` asserts WCAG AA under a REAL gtk3 theme in BOTH polarities** — ⚠️ **it SKIPS loudly rather than passing vacuously if the gtk3 plugin is absent.** ✅ **Result: dark 1.07:1 → 7.81:1; light 1.03:1 → 8.37:1; attention 8.02:1 / 5.67:1. All PASS AA.** | **Medium** | ⚠️ **Unassigned** — found by T-0496 live pass | ✅ **VERIFIED 2026-09-02 (user-approved)** — ⚠️ **verified the only way this defect CAN be: by a human looking at a real themed screen** |

---

## I-0183 — 🔴 DATA LOSS: an available-but-UNREADABLE world licensed a prune

⚠️ **The most serious Issue found by this pass, and the sprint's own risk table
believed it was already mitigated.** §6 listed *"Relink accepting the wrong
package"* → *"the CORE verifies `worldID`"*. ⚠️ **The verification EXISTS and RAN.
It is the WRONG CHECK: a COPY of a world carries the SAME `worldID`.**

⚠️ **But the copy was not the mechanism either.** `repairDangling` runs on **project
open**, not on relink — ⚠️ **so the relink destroyed nothing; the RELOAD did.**

### ⚠️ The actual chain

1. `WorldStore::resolve` reads **`world.json`** — SMALL, still served from the SMB
   client cache — so the world reported **`available`**.
2. **`index.json`** is larger and failed **`EBADF`** on the phantom mount.
3. `loadWorldIndex` fell back to `scanDir`, ⚠️ **which swallows every I/O failure
   with `continue`** — ✅ correct for building an index — ⚠️ **but it makes an
   UNREADABLE tree and an EMPTY one produce IDENTICAL results.**
4. The `available` branch `continue`d **without consulting the cached index**, so
   the endpoint came back `found=false, worldPending=false` = ⚠️ **`dangling`** =
   licensed to prune.

⚠️ **The prune logic itself was already CORRECT** — *"PENDING WINS OVER DANGLING,
unconditionally… Absence is never deletion."* ⚠️ **The hole was upstream.**

### ✅ The fix

1. `scanDir` reports `sawIOError`. ⚠️ **A failed `exists()` is "I could not tell",
   NOT "no such directory".**
2. `loadWorldIndex` reports `indeterminate` ⚠️ **and REFUSES to persist a scan it
   could not trust** — ⚠️ **writing that empty index into the package would have
   made the loss PERMANENT, outliving the mount that caused it.**
3. `ResolvedEndpoint::worldIndeterminate` — an available-but-unreadable world now
   yields **`pending`**, never `dangling`.

### ✅ How it was verified

⚠️ **Against a CONTROL, not by assertion.** The same real project data and the same
unreadable world, opened through `scrivi_open_project` on two cores built from the
same tree differing only in this fix:

| Core | Live edges | Log |
| ---- | ---------- | --- |
| ⚠️ **Unfixed** | ⚠️ **12 → 0** | 3620 → 4664 B (tombstones appended) |
| ✅ **Fixed** | ✅ **12 → 12** | byte-identical |

✅ **Then verified FROM THE WRITER'S SEAT 2026-09-02**: the user renamed the package
to recreate the trigger and ✅ **saw *"Objects held pending"*, not *"missing"***;
relinking cleared the notices; ✅ **data intact (12 and 46 live edges).**

✅ **Regression test `[I-0183]`** in `RelationshipTests.cpp`, ⚠️ **verified FAILING
against the unfixed core.** ✅ **User's lost data was restored** from a same-`projectID`
copy after checking the restore was a strict superset.

---

## I-0185 — a package must be an ATOM in the app

⚠️ **The user stated the rule and it is not negotiable:**

> *"The writer must perceive the folder structure as if it were a single file…
> in the app it must appear as if it were an atomic thing."*

⚠️ **This Issue was CORRECTED BY THE USER TWICE, and both corrections mattered.**

**Correction 1** — it is not a "picker" but ⚠️ **the standard file dialog, at THREE
sites**, including ⚠️ **`Open Project`, the app's most common action.** The original
filing described one Worlds affordance.

**Correction 2** — ⚠️ **which invalidated the first fix entirely.** ⚠️ **Single-click
+ "Choose" ALWAYS worked and was never the complaint.** ⚠️ **The complaint is the
DOUBLE-CLICK**, which descends and leaves the writer inside the package looking at
`manuscript/`, `objects/`, the `.json` files.

### ⚠️ The first fix was wrong, and passed all its own tests

⚠️ **It let the dialog descend and then resolved the RETURN VALUE back up to the
package.** ⚠️ **That corrects the path while leaving the writer standing inside the
package — it did not address the complaint at all.** ⚠️ **And it passed every
pure-function test written for it**, which is why the smoke now drives a REAL
`QTest::mouseDClick` through the actual dialog.

### ✅ The fix that works

A **non-native** `QFileDialog` with an **event filter on both item viewports** that
⚠️ **swallows the double-click on a package and CHOOSES it** — so ⚠️ **navigation
never happens.**

⚠️ **Three Qt obstacles, each recorded in the header:**

1. ⚠️ `accept()` / `done()` are **protected** on `QFileDialog` → a small
   `ChooserDialog` subclass exists only to expose closing.
2. ⚠️ **`selectFile()` does NOT work in Directory mode** — `selectedFiles()` reports
   the VIEWED directory, so a double-clicked package came back as its **parent**.
   ⚠️ **This silently broke the first attempt**; an explicit `chosen` member fixes it.
3. ⚠️ The filter must sit on the **viewport**, not the view — item views deliver
   mouse events there.

✅ **Accept button relabelled "Choose".** ⚠️ **NOT applied to "choose where to CREATE
a project"** — the writer is picking an ordinary parent folder and descending is
correct there. ✅ `packageRootFor` kept as a backstop for a typed path.

⚠️ **INVISIBLE ON APPLE** — Cocoa treats a package as an opaque bundle, so a
double-click selects it. ⚠️ **Every Qt platform re-earns this; Windows is next**
(`Scrivi_Platform_Porting_Outline_v0_1.md`).

✅ **`package_folder_smoke` drives a real double-click and is verified to FAIL
without the filter** — it times out, because the dialog descends and never returns.

---

## I-0184 — the Worlds row's path was CLIPPED, not elided

⚠️ **The first diagnosis was wrong and is recorded that way deliberately.** The
hardcoded 360 px elide was blamed; ✅ **measurement showed real world paths are
230–278 px and never reached it.** ⚠️ **The row LAYOUT was the cause:** the status
label and buttons took their natural widths, leaving the path label **202 px** for
a **211 px** path. ⚠️ **Eliding against 360 px when the label holds 202 px means
the text is not elided at all — so Qt CLIPS it, and clipping removes the TAIL**,
which is the package name that identifies the world.

✅ **Fix:** elide against the label's own `contentsRect().width()`, ⚠️ **at PAINT
time as well as on resize** — ⚠️ **a widget laid out but never shown gets no resize
event, so the first paint would still be unelided** (⚠️ **that gap was in the first
attempt at the fix; the new smoke caught it**) — plus size policies so fixed-width
siblings stop taking the path column's slack.

✅ **Smoke:** `worlds_path_smoke` asserts at the measured 202 px.

---

## I-0186 — `palette(mid)` is a STRUCTURAL role, used as a TEXT colour

⚠️ **Found by the user in ten seconds by looking at their own screen**, after a
long exchange in which ⚠️ **every automated check and a screenshot I offered as
evidence said the text was fine.**

| Theme | Path colour on background | Contrast |
| ----- | ------------------------- | -------- |
| Yaru-dark | `#2f2f2f` on `#2a2a2a` | ⚠️ **1.07:1** |
| Yaru light | `#ffffff` on `#fcfcfc` | ⚠️ **1.03:1** |

⚠️ **WCAG AA needs 4.5:1, so the path was invisible in BOTH real themes** — the
user reported dark, but light was no better. ⚠️ **`palette(link-visited)`, used as
the warning colour, resolved to MAGENTA `#ff00ff`.**

### ⚠️ Why every check missed it — the part worth keeping

⚠️ **Qt's NO-THEME fallback renders `Mid` as a pleasant `#b8b8b8` grey.** ⚠️ **Every
offscreen smoke, and the screenshot produced to demonstrate the I-0184 fix, used
that fallback** — so they showed a perfectly readable path that ⚠️ **no real user
ever saw.** ✅ **A HEADLESS RENDER IS NOT EVIDENCE ABOUT COLOUR.**

✅ **Fix:** shared `platforms/linux/src/ThemeColours.hpp` — ⚠️ **derive secondary
text by blending `WindowText` toward `Window`** (the two colours a theme
guarantees are legible against each other) ⚠️ **rather than naming any role**;
applied via the **palette**, not a stylesheet, on both the Active and Inactive
groups. ⚠️ **Also removed `SceneInspector`'s `setEnabled(false)`-as-dimming**,
which routed the label through the DISABLED group and overrode the colour.

✅ **Hoisted, not restated** — it affected both `WorldsDialog` and
`SceneInspector`, following the `WorldStatusText` precedent.

✅ **Smoke:** `theme_contrast_smoke` asserts WCAG AA under a REAL gtk3 theme in
both polarities, and ⚠️ **SKIPS loudly rather than passing vacuously** when the
gtk3 plugin is absent.

✅ **Result: dark 1.07:1 → 7.81:1; light 1.03:1 → 8.37:1.**

### ⚠️ The smoke SKIPS over plain SSH — deliberately, and this is not a gap to "fix"

⚠️ **The gtk3 platform theme needs a DISPLAY even under `QT_QPA_PLATFORM=offscreen`** — GTK opens one
itself to read the theme and dies with *"cannot open display"* otherwise. ⚠️ **So the smoke passes
when run from inside the RDP session and FAILED over plain SSH**, which is how `deploy-to-rig.sh`
runs it. ⚠️ **Observed 2026-09-03**, one build after the fix landed.

✅ **It now SKIPS LOUDLY** (using `xvfb-run` when present), naming the reason.

⚠️ **DO NOT "fix" this by dropping the gtk3 theme.** ⚠️ **The test would then measure Qt's no-theme
fallback — which is the EXACT blind spot this Issue exists in.** ⚠️ **A vacuous pass here is worse
than no test at all**, because it would certify the contrast of a palette no user has.

⚠️ **CONSEQUENCE worth knowing: the contrast check does NOT run in the normal deploy loop.** ✅ **To
exercise it, run the smoke from inside the RDP session, or install `xvfb` on the rig.**
