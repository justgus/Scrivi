# Verified Tasks — T-0554 · T-0563 · T-0564

**Issues closed:** ✅ **[I-0252]** (⚠️ **both platform halves**) · ✅ **[I-0256]**
**Platform:** ⚠️ **`[Apple]` (T-0554) and `[Linux]` (T-0563, T-0564)**
**Sprint:** ⛔ **NONE — ✅ all three taken standalone**, ⚠️ **deliberately: the writer was impeded and
[SP-146] was closing.**
**Implemented:** 2026-09-29 · ✅ **USER-VERIFIED 2026-09-29 by live pass**
**Archived:** 2026-09-29 (`feedback_archive_on_close`).

---

## ✅ The verification

✅ **THE USER, on macOS:** ***"on macOS the line is now fully visible."***
✅ **THE USER, on the Linux rig:** ***"I verified in the live rig that the scene divider now displays
perfectly in Linux."*** ⚠️ **The Navigator toggle (`Ctrl+Alt+N`) was exercised in the same pass.**

⚠️ **AND THE USER FOUND THE LINUX HALF BY REPORTING THE MACOS FIX** — ✅ ***"however, on Linux it is
only barely visible in dark mode"*** — ⛔ **which no suite had caught, because `theme_contrast_smoke`
did not cover the divider and its wrapper was silently skipping on this architecture entirely.**

---

## ✅ T-0554 — `[Apple]` ✅ **Divider visibility ([I-0252])** — ✅ **FIXED 2026-09-29 (third attempt), ✅ **VERIFIED 2026-09-29****

⛔ **THE CAUSE WAS FOUND BY MEASUREMENT, AND IT WAS NEITHER OF THE FIRST TWO DIAGNOSES.**
✅ **MEASURED by rendering a REAL `NSTextView` to a bitmap and sampling it** — ⚠️ **not a screenshot,
⛔ not documentation:**

| Probe | Result |
| ----- | ------ |
| engine | ✅ **TextKit 2** |
| `attachmentBounds` calls | ✅ **1 — ⚠️ the 24 pt gap IS reserved** |
| ⛔ **`image(for:)` calls** | ⛔ **ZERO** |
| ⛔ **an OPAQUE MAGENTA bar on the canvas** | ⛔ **0 pixels** |

⛔ **THE LINE WAS NEVER BEING DRAWN AT ALL** — ⚠️ **TK2 reserved its height and drew nothing, ✅ which
presents as a GAP between scenes, not a faint line.** ⛔ **So no colour could ever have fixed it.**

### ✅ THE FIX — one line

```swift
attachment.image = NSImage(size: NSSize(width: 1, height: 1))
```

⚠️ **A placeholder `image` PROPERTY makes TK2 take the image path; ✅ the `image(for:)` override then
supplies the real, correctly-width-ed art per layout pass** (⚠️ measured: `image(for:) calls = 2`).
⚠️ **1×1 is deliberate — ⛔ the override always replaces it, so the placeholder's size is never used
and must not be mistaken for the divider's geometry (`attachmentBounds` owns that).**

### ✅ PROVEN WITH THE APP'S REAL DRAWING CODE, ⛔ not the probe

⚠️ **Sampling a column PAST the text so glyphs cannot be counted:**

| Appearance | ⛔ without the fix | ✅ with the fix |
| ---------- | ------------------ | -------------- |
| **DARK** | ⛔ **0 drawn rows** | ✅ **2** (⚠️ 1 pt stroke + one antialiased row) |
| **LIGHT** | ⛔ **0 drawn rows** | ✅ **2** |

✅ **THE TWO EARLIER FIXES WERE KEPT AND ARE NOT WASTED** — ⚠️ **`secondaryLabelColor` (`5.89 : 1`
Dark) and the half-pixel alignment were correctly measured; ⛔ they were invisible behind a line that
never drew.** ✅ **Now that it draws, they are what make it READABLE.**

### ⚠️ Why this took three attempts — ✅ the lesson

⛔ **1:** [T-0526] dropping [I-0112]'s appearance guard. ✅ **Disproven by measurement.**
⛔ **2:** `separatorColor` at `1.34 : 1`. ✅ **Correct arithmetic, ⛔ wrong layer.**
✅ **3:** *"does it draw AT ALL?"* — ⚠️ **the question [EP-045]'s design doc said to ask FIRST.**
⚠️ **`feedback_prove_code_is_reached`: *"it didn't change anything" meant it wasn't running.*** ⛔ **The
same class, twice, on one Issue.**

✅ **VERIFIED:** ⚠️ **`xcodebuild -scheme ScriviApp` → BUILD SUCCEEDED** · ✅ **pixel measurement, both
appearances.** ⛔ **`xcodebuild test` NOT RUN** — ⚠️ **[I-0150]: the runner LAUNCHES the app and once
rewrote a real project.** ✅ **View-layer drawing, no ScriviCore involvement; ⚠️ the pixel measurement
is the stronger evidence anyway.**
⛔ **NOT user-Verified — ✅ the user is in Dark Mode and can confirm by eye.**
⚠️ **STILL SUPERSEDED BY the ruled CONFIGURABLE GLYPH** — ✅ **this restores VISIBILITY only.**
✅ **[EP-045] AC2's diagnostic obligation is DISCHARGED.**

---

## ✅ T-0563 — `[Linux]` **Scene Navigator show/hide ([I-0256])** — ✅ **Implemented 2026-09-29, NOT VERIFIED**

⚠️ **NO SPRINT** — ✅ **taken standalone; ⛔ deliberately NOT folded into [SP-146]**, which closed the
same day. ⚠️ **It is a small pane fix with its own proof, ✅ and [SP-146]'s ACs were already met.**

✅ **`View ▸ Show Scene Navigator` (`Ctrl+Alt+N`)** — ⚠️ **joins `Ctrl+Alt+I` (inspector) and
`Ctrl+Alt+T` (timeline).** ✅ **The Ctrl+Alt family is deliberate:
`project_linux_vnc_input_constraints` records that the macOS→VNC path EATS Ctrl+Shift combos.**

✅ **THE USER'S RULED MECHANISM, AND IT NEEDED NO NEW MACHINERY** — ⚠️ ***"you can set its slider width
to 0. That will be adequate."*** ⛔ **`setCollapsible(2, false)` is applied to the INSPECTOR ONLY
(`EditorShell.cpp:160`), ✅ so the navigator at index 0 has been collapsible all along** — ⚠️ **the work
was a menu item and a per-window check-state sync, ⛔ not a mechanism.**

✅ **MEASURED, ⛔ not assumed** (throwaway harness, 8/8 PASS):

| | Result |
| - | ------ |
| navigator pane width, shown → hidden | ✅ **`240 → 0` px** |
| viewport width | ✅ **RECLAIMS `572 → 816`** |
| restore | ✅ **back to `240`** |
| `isCollapsible(0)` / `isCollapsible(2)` | ✅ **true / false — ⚠️ the inspector's guard is untouched** |

⚠️ **A HARNESS ERROR OF MINE, recorded:** ⛔ **the first run FAILED the two width checks** — ✅ **because
`findChild<QSplitter*>()` returned the OUTER VERTICAL splitter (panes over timeline), not the inner
three-pane row.** ⚠️ **Qt said so plainly (*"isCollapsible: Index 2 out of range"*) and I nearly read
it as a code defect.** ✅ **Fixed by selecting the horizontal splitter with 3 panes.**

⛔ **SESSION-SCOPED, NOT PERSISTED — ⚠️ deliberately, and NOT an omission.** ✅ **[I-0255] ruled
TIMELINE visibility should persist and is a `[Cross]` defect on both platforms; ⛔ the Navigator has had
no such ruling, ⚠️ and Apple's `columnVisibility` is `@State` (per-view, in-memory)** — ✅ **so
persisting Linux's alone would INVENT a parity gap rather than close one.** ⚠️ **[T-0556]'s focus mode
is where all three panes get settled together.**

✅ **VERIFIED BY RUNNING:** ⚠️ **`ctest` 641/641 NON-ROOT** · ✅ **smokes 23/23** · ✅ **nav harness 8/8**
· ✅ **boundary GREEN.** ⛔ **NOT user-Verified — ⚠️ needs the rig.**

---

## ✅ T-0564 — `[Linux]` **Scene divider visibility ([I-0252] Linux half)** — ✅ **Implemented 2026-09-29, NOT VERIFIED**

⚠️ **USER, after the macOS fix landed:** ***"on macOS the line is now fully visible. However, on Linux
it is only barely visible in dark mode."***

⛔ **A SEPARATE IMPLEMENTATION AND A SEPARATE DEFECT** — ✅ **Linux paints the rule itself in
`ManuscriptEditor::paintEvent`; ⚠️ it was never affected by the TextKit 2 attachment bug [T-0554]
fixed.**

### ⛔ THE CAUSE — ✅ a role this project had ALREADY measured as invisible

⚠️ **The divider used `palette().color(QPalette::Mid)`.** ⛔ **`Mid` is a STRUCTURAL role with NO
contrast guarantee** — ✅ **and [I-0186] MEASURED it at `1.07:1` on Yaru-dark in 2026, which is why
`ThemeColours.hpp` exists and says in its own header: *"NEVER name a palette role for text colour."***
⚠️ **The divider's comment even called `Mid` *"the Qt analogue of NSColor.separatorColor"*** — ⛔ **and
`separatorColor` is exactly what [I-0252] removed on macOS for the same reason.**

✅ **MEASURED 2026-09-29 UNDER REAL GTK THEMES** (⚠️ **not Qt's fallback —
`ThemeColours.hpp` is explicit that a headless no-theme render is NOT evidence about colour**):

| | Yaru-dark | Yaru light |
| - | --------- | ---------- |
| body text (⚠️ the scale) | `13.62:1` | `21.00:1` |
| ⛔ **old `palette(Mid)`** | ⛔ **`1.03:1`** (`#262626` on `#242424`) | ⛔ **`1.98:1`** |
| ✅ **new `ThemeColours::rule()`** | ✅ **`3.93:1`** | ✅ **`3.36:1`** |

⚠️ **LIGHT MODE WAS WRONG TOO** — ✅ **the user noticed it in dark, ⛔ but at `1.98:1` it was faint in
both, exactly as [I-0186] was.**

### ✅ THE FIX

✅ **A new `ThemeColours::rule()`, DERIVED from `WindowText` on `Base`** — ⚠️ **`Base`, not `Window`,
because the manuscript is a `QPlainTextEdit` and its background is the text-entry role.**
⚠️ **Deliberately fainter than `deemphasised()` (55% vs 30% blend): ⛔ a divider is a MARK, not text,
and must not compete with prose** — ✅ **macOS settled the same band by measurement
(`secondaryLabelColor` `5.89:1` against body text's `16.67:1`).**

### ✅ A REGRESSION GUARD, ⛔ AND IT WAS PROVEN BY BREAKING IT

✅ **`theme_contrast_smoke` now checks `rule()` against `Base` under BOTH real theme polarities**, ⚠️ **at
a `3.0:1` floor rather than WCAG AA's `4.5:1`** — ⛔ **AA is a TEXT threshold and does not apply to a
structural mark.**
✅ **PROVEN RED: `rule()` was temporarily reverted to `palette(Mid)` and the smoke FAILED with
*"the scene divider is 1.03:1 … needs 3.0:1"***, ⚠️ **then green on restore.**

### ⛔ AND THE GUARD ITSELF HAD A HOLE — ✅ found while using it

⛔ **`theme_contrast_smoke.sh` tested for `libqgtk3.so` at a HARDCODED `x86_64-linux-gnu` path**, ⚠️ **so
on every **aarch64** machine — including the ARM Docker image used for development — it SKIPPED
SILENTLY with exit 0.** ✅ **Fixed to an arch-agnostic glob.** ⚠️ **A guard that skips on the
developer's own architecture is not a guard** — ⛔ **it is the same blind spot [I-0186] lived in, one
level up.**

✅ **VERIFIED BY RUNNING:** ⚠️ **`ctest` 641/641 NON-ROOT** · ✅ **smokes 23/23** · ✅ **theme contrast
PASS both polarities (⚠️ under real Yaru/Yaru-dark, gtk3 + xvfb)** · ✅ **Docker build 0 warnings** ·
✅ **boundary GREEN.**
⛔ **NOT user-Verified — ⚠️ needs the rig in dark mode.**


---

## ⚠️ THE ARC — ✅ four attempts, two platforms, two unrelated causes

| # | Platform | ⚠️ Theory | Outcome |
| - | -------- | -------- | ------- |
| 1 | macOS | ⚠️ a dropped appearance guard ([T-0526] vs [I-0112]) | ⛔ **DISPROVEN by measurement** |
| 2 | macOS | ⚠️ `separatorColor` is chrome at `1.34:1` | ⛔ **SHIPPED, CHANGED NOTHING** |
| 3 | macOS | ✅ **does it draw AT ALL?** | ✅ **THE ANSWER — `image(for:)` called ZERO times** |
| 4 | Linux | ✅ **`palette(Mid)` has no contrast guarantee** | ✅ **THE ANSWER — `1.03:1` on Yaru-dark** |

✅ **THREE LESSONS, ALL PRE-EXISTING RULES:**
1. ⛔ **`feedback_prove_code_is_reached`** — ⚠️ **two fixes adjusted the colour of a line that was never
   drawn.** ✅ **[EP-045]'s design doc said to ask *"does it draw?"* FIRST.**
2. ⛔ **`feedback_look_for_existing_pattern_first`** — ⚠️ **`ThemeColours.hpp` already said *"NEVER name
   a palette role for text colour"* and already recorded `Mid` at `1.07:1`.**
3. ⛔ **A guard that skips on the developer's own architecture is not a guard** — ✅ **fixed in
   [T-0564].**

⚠️ **STILL SUPERSEDED:** ✅ **the scene break becomes a CONFIGURABLE GLYPH under the manuscript-renderer
Epics.** ⛔ **These restored VISIBILITY only.**
✅ **[EP-045] AC2's diagnostic obligation is DISCHARGED.**
