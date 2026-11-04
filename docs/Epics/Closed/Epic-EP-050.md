## EP-050: `[Apple]` ⚠️ **Manuscript Accessibility** — VoiceOver reads the page the writer reads

**Status:** ✅ **CLOSED 2026-11-04** (user, 2026-11-04: *"approved."*) — ✅ AC1–AC7 met; Sprints [SP-171], [SP-172], [SP-202], [SP-203] closed; ✅ Audit Check run at the close (findings below). — activated 2026-10-08 (SP-171 activated) — created 2026-10-08 (user: *"let's look at I-0277"*; rulings A1–A3 the same day). ✅ S1 = [SP-171], ✅ closed 2026-10-08; S2 = [SP-172], ✅ closed 2026-10-09. S3 = [SP-202] and S4 = [SP-203], ✅ closed 2026-11-04.
**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-171]** | **S1** — the translation layer (one map, shared with Find), the caret both ways, cost: AC1, AC2, AC3 + AC5 reading half | ✅ **CLOSED 2026-10-08** (user-approved) → [`../Sprints/Closed/Sprint-SP-171.md`](../../Sprints/Closed/Sprint-SP-171.md) · Q2 re-ruled: dividers read as WORDS · ✅ AC1, AC2, AC3, AC5 reading half met |
| **[SP-172]** | **S2** — the Headings rotor: AC4 + AC5 rotor half | ✅ **CLOSED 2026-10-09** (user-approved) → [`../Sprints/Closed/Sprint-SP-172.md`](../../Sprints/Closed/Sprint-SP-172.md) · rulings R1–R3 there · ✅ AC4 + AC5 rotor half met (live pass 2026-10-09); T-0600 verified |
| **[SP-202]** | **S3** — Linux: AC6 (measure what Orca is given, then rule) | ✅ **CLOSED 2026-11-04** (user-approved) → [`../Sprints/Closed/Sprint-SP-202.md`](../../Sprints/Closed/Sprint-SP-202.md) · ✅ measured: Orca was given the STORED text · ✅ Q1–Q3 ruled (fix here, S4) |
| **[SP-203]** | **S4** — Linux: AC7 — Orca reads the page at rest (Apple's AC1–AC3 on Linux) | ✅ **CLOSED 2026-11-04** (user-approved) → [`../Sprints/Closed/Sprint-SP-203.md`](../../Sprints/Closed/Sprint-SP-203.md) · ✅ built, scrivi-vm + Docker green · ✅ Q1 ruled (headings: recorded, nothing built) · listen → [T-0660] |

**Tasks:** ✅ [T-0599] (SP-171 — VERIFIED, archived) · ✅ [T-0600] (SP-172 — VERIFIED, archived) · ✅ [T-0658] (SP-202) · ✅ [T-0659] (SP-203) · ✅ [T-0660] ([EP-057]: the Orca pass) — all VERIFIED 2026-11-04, archived.
**Issues:** ✅ [I-0277] `[Apple]` VoiceOver reads the STORED manuscript — fixed and VERIFIED in [SP-171], archived. · 🔵 [I-0284] (found in SP-171 → Issue backlog).
**Authority:** design [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §13 (*"VoiceOver /
accessibility reads storage. Hidden markers may be spoken; not checked."*); SDK `AppKit/NSAccessibilityProtocols.h`,
`NSAccessibilityCustomRotor.h` (read 2026-10-08).

**Goal:** ✅ **A writer using VoiceOver hears the manuscript as it reads on the page** — no backslashes, no `##`, no `**` — and can
move from heading to heading the way she would on the web.

### ⚠️ What the SDK says the work is (read 2026-10-08)

A text view speaks to assistive technology through **~15 members, every one in CHARACTER POSITIONS**: `accessibilityValue`,
`accessibilityNumberOfCharacters`, `accessibilitySelectedText`, `accessibilitySelectedTextRange(s)` (get AND set — VoiceOver moves the
caret through them), `accessibilityVisibleCharacterRange`, `accessibilityInsertionPointLineNumber`, `accessibilityLine(for:)`,
`accessibilityRange(forLine:)`, `accessibilityString(for:)`, `accessibilityAttributedString(for:)`, `accessibilityRange(forPosition:)`,
`accessibilityRange(for index:)`, `accessibilityFrame(for:)`, `accessibilityRTF(for:)`. ⛔ So the fix is not "a different string": it is a
**TRANSLATION LAYER** between the PRESENTED text and STORAGE, consistent across every member — ⚠️ or VoiceOver's cursor and Scrivi's
caret drift apart (the [I-0282] lesson). ✅ The pieces exist: `MarkdownEscapes.map`, the presenter's `stopTest` / `isHidden`, the
caret-snap rules. ✅ `NSAccessibilityCustomRotor` (macOS 10.13+) has `NSAccessibilityCustomRotorTypeHeading` and a text result's
`targetRange` — the Headings rotor.

### ✅ Rulings — 2026-10-08 (user)

| # | Question | Ruling |
| - | -------- | ------ |
| **A1** | What text VoiceOver gets | ✅ **The page AT REST** — every Markdown mark hidden (escapes, heading `#`, emphasis markers) whatever the caret or Markup Hints; list prefixes kept, as on screen. Stable as the caret moves |
| **A2** | Headings | ✅ **A Headings rotor** — VoiceOver moves heading to heading (and chapter titles) |
| **A3** | Tracking | ✅ **This Epic**; [I-0277] its first Issue; Linux its own criterion |

### Acceptance criteria

| AC | Criterion |
| -- | --------- |
| **AC1** | ✅ **MET** ([SP-171]) — **One translation layer (A1):** the accessibility text is the page at rest; EVERY member above maps presented ⇄ storage through ONE map; a corpus test proves round trips and agreement between members (e.g. `string(for: r)` = the substring of `value` at `r`; `line(for:)` / `range(forLine:)` agree) |
| **AC2** | ✅ **MET** ([SP-171]) — **The caret, both ways:** a selection set by VoiceOver lands where Scrivi's caret rules put it (never inside hidden markup); Scrivi's caret is reported in presented positions |
| **AC3** | ✅ **MET** ([SP-171]) — **Cost:** the map is cached and invalidated per edit — measured on 1.8 MB (`numberOfCharacters`, `string(for:)`, a keystroke with VoiceOver's queries); recorded |
| **AC4** | ✅ **MET** ([SP-172]) — **The Headings rotor (A2):** Markdown headings and chapter titles, in order, each a `targetRange` in presented positions; next/previous from **VoiceOver's reading position** (amended 2026-10-08, SP-172 R3: VoiceOver never passes the caret) |
| **AC5** | ✅ **MET** ([SP-171] (reading) + [SP-172] (rotor)) — **Live pass with VoiceOver** (user, Mac): it reads the page without markup at default punctuation verbosity; the rotor moves heading to heading; typing with VoiceOver on behaves |
| **AC6** | ✅ **MET 2026-11-04** ([SP-202]) — **Linux** (Qt accessibility reads the stored text too, I-0277): MEASURE what Orca hears, then rule — a criterion here or EP-048's. ✅ Measured; ✅ ruled: **here**, as AC7 |
| **AC7** | ✅ **MET 2026-11-04** ([SP-203]; [T-0660] Orca pass) — ✅ **ADDED 2026-11-04** ([SP-202] Q1–Q2) — **Linux: Orca is given the page at rest** (A1): every text member through the one map, the caret both ways, cost recorded (Apple's AC1–AC3) — [SP-203]; heading navigation measured, then ruled ([SP-203] Q1); the Orca listen [T-0660] ([EP-057]) |

⛔ **OUT:** iOS/visionOS (no manuscript surface yet) · spoken formatting ("bold") beyond what the attributed string carries.

### ✅ Audit Check at the close (2026-11-04) — ruled by the user ("approved")

| # | Finding | Ruling |
|---|---|---|
| F1 | `Epic-Documentation.md`'s EP-050 row was a month stale ("S1 = [SP-171] (🟡 active)", "AC1–AC6") | ✅ Fixed at the close |
| F2 | AC1–AC5 carried no met mark in the AC table (only in the Sprint table); AC7 was not marked met | ✅ Fixed at the close (above) |
| F3 | [I-0277] and [I-0282] have batch-table rows but no entry sections (`Issue-verified-0271-0280.md`: 8 rows / 1 entry; `0281-0290`: 9 / 6) | ✅ Accepted for this close (both verified; the rows say so); a full Audit of the Issue archives recommended — only on the user's request |
| F4 | (outside EP-050) EP-048 listed [I-0280] 🟠 and [I-0281] 🔵 backlog, both verified and archived | ✅ Fixed (user: *"also fix the documentation error in EP-048 as well"*) |

Clean: Sprint status (all four closed everywhere), Task evidence (T-0599, T-0600, T-0658–T-0660 archived), no restated counts,
no orphan files, ID continuity (next SP-204, T-0662, I-0346).

