---
sprint: SP-182
epic: EP-048
status: Active
activated: 2026-10-15
platform: Linux
created: 2026-10-15
---

# SP-182 — `[Linux]` [EP-048] **S5a**: Project Settings dialog, the typeface, Markup Hints

**Status:** 🟡 **ACTIVATED 2026-10-15** (user: *"activate SP-182"*) — created 2026-10-15 (user: *"plan EP-048 fully."*). ✅ Q1–Q5 ruled 2026-10-15, all as
recommended; the S5 split is ruled (Q2).
**Carries:** [T-0609] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-048] `[Linux]` Manuscript Renderer Parity → [`../Epics/Epic-active.md`](../Epics/Epic-active.md). **L10, its
character half:** the project typeface and size, and Markup Hints on/off. ✅ [EP-026] Q4 (ruled 2026-10-11): **EP-048 S5
builds Linux's Project Settings dialog**, and EP-026 S3 later adds its Undo History section.
**Authority:** [EP-047] P1, P5–P9, P11 and AC4, AC5, AC7 (`../Epics/Closed/Epic-EP-047.md`) · `Resources/Fonts/fonts.json`
(*"THE list of bundled manuscript typefaces — read by the Apple app, the Linux app (EP-048 L10) … Never restate it in
code"*) · `docs/Scrivi_Project_Package_Structure_v0_1.md` (`project-settings.json`).
**Shape:** ✅ Apple's (`feedback_linux_adopts_apple_shape`): `ProjectSettingsSheet.swift`, `ProjectPreferences.swift`,
`ManuscriptTypography.swift`, `BundledFonts`, and View ▸ Show Markup Hints (`ScriviApp.swift:440`).
**Size:** M. ⚠️ Needs a **rig live pass**.

---

## What reading the code found (2026-10-15)

- ⛔ **Linux reads no project setting.** Project ▸ Project Settings… is still the T-0312 stub, a dialog saying *"Project
  settings are coming soon."* (`platforms/linux/src/ScriviWindow.cpp:382–397`).
- ✅ **The bridge is done** ([SP-167]): `getProjectSettings`, `putProjectSettings`, `setProjectTitle`
  (`ScriviBridge.cpp:783–813`). ⚠️ Only `bridge_parity_smoke` calls them; the app never does.
- **Apple's persistence** (`ProjectPreferences.swift`):
  - The whole document is kept and written back, so a key another build wrote is never dropped ([I-0215]).
  - A typography key is written only once the writer has chosen it, so saving another setting never pins today's default.
  - Absent values are defaults: Literata (P6), 16 pt (P9), hints ON (P11).
  - The title goes to `project.json` through `scrivi_set_project_title`, and an empty title is not written.
  - An unreadable file is never overwritten on open; only an explicit change by the writer replaces it.
- **Apple's sheet** (`ProjectSettingsSheet.swift`) has four sections:
  - Project: Title, Subtitle.
  - Writing Surface: Show chapter titles.
  - Typography: Typeface, Text size (10–32, step 1), Paragraph indent, Indent (0.5–4 em, disabled at None).
  - Undo History.
  Changes apply live; *Done* closes it.
- ⛔ **Linux bundles no font.** The editor draws in the system default (nothing calls `setFont` on `ManuscriptEditor`). The
  presenter sizes headings from `document()->defaultFont()` (`ManuscriptPresenter.cpp:281, 319`), so a face and size set as
  the document's default font reach the headings and emphasis with no new path.
- ⚠️ **The faces are VARIABLE fonts** (`Literata[opsz,wght].ttf` and the like; only Courier Prime ships static files). How
  Qt picks bold and italic from a variable TTF registered with `QFontDatabase::addApplicationFont` is unmeasured.
  `QFont::setVariableAxis` arrived in Qt 6.7. Docker builds on Ubuntu 24.04 (**Qt 6.4.2**), while the rig runs **Qt 6.10.2**
  (`docs/Scrivi_Linux_Rig_Setup_v0_1.md:285`), so it must be measured on both.
- ✅ **Markup Hints are a reveal switch.** The Linux presenter's reveal is one call (`ManuscriptPresenter::reveal`,
  `ManuscriptPresenter.cpp:187`; called from `ManuscriptEditor.cpp:73, 97`). [SP-170] found on Apple that hints-OFF's caret
  rules already held, because a stop run's home does not depend on the reveal. ⏳ Check the same on Linux, not assume it.
- ✅ The View menu exists (`ScriviWindow.cpp:296`). Apple's ⇧⌘H becomes **Ctrl+Shift+H** by the Ctrl-for-⌘ rule. Nothing in
  the Linux app binds it.

## Goal

✅ **A project looks the same on Linux as on the Mac**: its typeface and size, set from a real Project Settings dialog that
reads and writes the same `project-settings.json`, with Markup Hints on or off.

## Plan

0. **Measure the fonts first** (Docker Qt 6.4.2 and the rig's Qt 6.10.2). Register Literata from its variable file and
   render regular, **bold**, *italic* and bold-italic. Record whether each comes from the font's own axis or is synthesized,
   with `QFontInfo` and an offscreen render. If Qt 6.4 cannot select the weight, record it and rule (it decides whether the
   Docker smoke can assert the weight).
1. **`ProjectPreferences` (Linux, Apple's shape):** one per project window. It reads `getProjectSettings` at load, keeps the
   whole document, writes only the keys that were chosen, and sends the title through `setProjectTitle`. An unreadable file
   is not overwritten on open.
2. **`BundledFonts` (Linux):** read `fonts.json` once at startup and register every file it lists. Shipped per **Q1**, with
   each family's `OFL.txt` (EP-047 AC5: *"the OFL license texts ship with the app"*). A stored face this build does not have
   is KEPT in the file and drawn in the default (Apple: `BundledFonts.face(named:)`).
3. **One typography source:** face and size become the manuscript document's default font. Headings and bold/italic follow
   through the presenter; chapter titles use Heading 1 size in the text colour, in the face (P7, P9). A change re-presents
   only: no document edit, no history event, no dirty scene, no change to any `.md`.
4. **The dialog** (Q2): Project (Title, Subtitle) and Typography (Typeface, each name drawn in its own face; Text size
   10–32). Changes apply live, as Apple's do; *Close* closes it. Only what works now is shown.
5. **View ▸ Show Markup Hints** (Ctrl+Shift+H, Q4): checkable, per project window, persisted as `markupHints` (absent = ON).
   OFF reveals nothing. A toggle re-highlights only: no edit, no history event.
6. **Tests:** a new `settings_smoke`, in CI from the start:
   - The dialog round-trips through the file: a key this build does not know survives, and an unchosen typography key is
     not written.
   - The title lands in `project.json`; an empty title is not written.
   - A file the Mac wrote is read as written.
   - Typography: save bytes unchanged; no history event; the face survives typing, Return, undo, reload, paste and a
     structured paste.
   - Hints OFF reveals nothing, and the caret rules hold.
   - Ctrl+Shift+H through the real dispatch.
   - **Mutations:** a dropped unknown key · a default pinned on another save · hints ignored · a face reset by reload.
7. **Live pass** (user, rig), including a typeface set on the Mac showing on Linux and one set on Linux showing on the Mac.

## Questions — ✅ Q1–Q5 RULED 2026-10-15 (all as recommended)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **How the fonts ship on Linux** (5.7 MB, 7 families, plus `OFL.txt` each). | ✅ **Compiled into the app as a Qt resource**, generated from `Resources/Fonts/` and `fonts.json` at build time, so the list stays the ONE list. It works the same from the build tree on the rig and in Docker, with no install path to resolve. Alternative: install them beside the binary (a `share/` path the app has to find at runtime) · ✅ **RULED** (user, 2026-10-15) |
| **Q2** | **Splitting S5, and the dialog's first contents.** L10 is four mechanisms: the type (character formats), the indent and line spacing (block formats, unmeasured on Qt), Markup Hints, and the chapter-titles toggle (a change to how `SceneDocument` is built). Apple took three Sprints for the first three ([SP-168]–[SP-170]). | ✅ **Split into S5a (this Sprint: dialog, type, hints) · S5b ([SP-183]: line spacing and indent) · S5c ([SP-184]: show chapter titles).** The dialog shows only what works now: S5b and S5c add their rows, and EP-026 S3 adds Undo History. Alternative: one S5, or the full sheet now with the unbuilt rows disabled · ✅ **RULED** (user, 2026-10-15) |
| **Q3** | **Line spacing (P9: fixed 1.45 × size).** It is part of the type on Apple, but on Qt a line height is a BLOCK format, like the indent. | ✅ **Move it to S5b**, measured with the indent, since both have the same unknowns. S5a draws at Qt's default line spacing meanwhile. Alternative: here, and measure block formats in this Sprint · ✅ **RULED** (user, 2026-10-15) |
| **Q4** | **The Markup Hints shortcut.** | ✅ **Ctrl+Shift+H** (Apple's ⇧⌘H, Ctrl in place of ⌘), measured against GNOME on the rig. Alternative: menu only · ✅ **RULED** (user, 2026-10-15) |
| **Q5** | **Existing Linux manuscripts change look.** P6 (*"Projects with NO typeface setting — existing ones included — open in the default"*) was ruled for the Mac. | ✅ **Apply it on Linux too**: a project with no setting opens in Literata, 16 pt, as it does on the Mac. The setting travels, so anything else would make one project look different on each machine · ✅ **RULED** (user, 2026-10-15) |

⛔ **NOT in this Sprint:** line spacing and the first-line indent (→ S5b, Q3) · Show chapter titles (→ S5c) · the Undo History
section (→ EP-026 S3) · installed (non-bundled) fonts (P5: bundled only) · per-scene type (P8).

## Acceptance Criteria

- [ ] AC0 — Plan step 0 measured and recorded on Qt 6.4.2 and Qt 6.10.2: regular, bold, italic and bold-italic from the
      variable files
- [ ] AC1 — Project ▸ Project Settings… is a real dialog: Title (to `project.json`) and Subtitle, read from and written to the
      same files as the Mac; the whole settings document is kept; an unchosen key is never written; an unreadable file is
      not overwritten on open
- [ ] AC2 — **L10 (type):** the seven bundled faces register from `fonts.json`, and their OFL texts ship; the manuscript,
      headings, chapter titles and bold/italic draw in the project's face and size; a face the build does not have is kept
      and drawn in the default; a change re-presents only (save bytes unchanged, no history event, no dirty scene)
- [ ] AC3 — **L10 (hints):** View ▸ Show Markup Hints, Ctrl+Shift+H, persisted (absent = ON); OFF reveals nothing and the
      caret rules hold; a toggle changes no text and records no step
- [ ] AC4 — `settings_smoke` in CI, mutation-checked; ctest green on macOS and Linux; all Linux smokes green
- [ ] AC5 — ✅ **Rig live pass (user)**, including a typeface crossing between the Mac and Linux in both directions

## Progress log

### 🔵 2026-10-15 — Sprint CREATED in Planning

User: *"plan EP-048 fully."* Issued with [SP-181], [SP-183] and [SP-184] via `next-id.py`. [T-0609] filed. Q1–Q5 await
rulings; Q2 also rules the S5 split behind [SP-183] and [SP-184].

### 🔵 2026-10-15 — Q1–Q5 RULED (all as recommended); ready to activate

User: *"SP-182 Q2 agreed. SP-182 Q5 and SP-184 Q1: agreed. SP-182 Q1: agreed. SP-182 Q3-Q4: agreed."* The S5 split
stands: [SP-183] and [SP-184] go ahead.
