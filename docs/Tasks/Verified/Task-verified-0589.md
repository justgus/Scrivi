# Verified Task — T-0589

**Sprint:** [SP-170] · **Epic:** [EP-047] (S4) · **Platform:** `[Apple]` (Linux half → EP-048 L10)
**Filed:** 2026-10-05 · **Implemented:** 2026-10-08 · ✅ **USER-VERIFIED 2026-10-08 by live pass** (*"1. passes. 2. passes. 3. passes. 4. passes. 5. passes."*)
**Archived:** 2026-10-08, with the Sprint close (*"close SP-170"*).

---

## ✅ T-0589 — `[Apple]` + `[Linux]` Markup Hints on/off (View menu; persisted in the project)

**Created:** 2026-10-05 (user: *"I think we will need to eventually make the feature \"show/hide markup\" as an optional feature that the writer chooses from the Project Settings"* · *"Yes file the feature as a backlog task linked to EP-047. What \"show\" will mean in this context is exactly what it does right now. I'm not adking you to show all the markup, just the \"Markup Hints\" that appear when the caret is right next to a markup."*)
**Epic:** **[EP-047]** (Typography & Preferences) — ✅ **AC7**; planned for S4 (2026-10-07) · **Sprint:** ✅ **[SP-170]** (activated 2026-10-08, user: *"yes please"* (to *"Shall I activate SP-170 and start?"*)) → [`../Sprints/Sprint-SP-170.md`](../../Sprints/Closed/Sprint-SP-170.md) · ⚠️ **Sequence AFTER [EP-046] E2-S2** (inline
markers), so the setting covers every kind of hint at once. **Linux:** the same preference under [EP-048].

**What it is — exactly, and no more:** ✅ **"Markup Hints" = the re-entry reveal** that EP-046 builds: the dimmed
markers that appear when the caret is on or next to markup (today: a heading's `## ` on the caret's line, [SP-161];
from E2-S2: `**`/`*` around the span the caret touches). **ON = today's behaviour. OFF = markers stay hidden even
when the caret is there.** ⛔ **NOT** a "show all markup" / raw-source mode: rendering is unchanged either way.

**Seam (as built in SP-161):** the reveal lives in ONE place, `ManuscriptPresenter.reveal(for:in:)` and the
`revealing:` argument of `hiddenTest`. With hints OFF the revealed set is empty. Linux: the `rehighlightBlock`
reveal of [EP-048] L6.

✅ **CARET RULES WITH HINTS OFF — RULED 2026-10-05 (user):** a hidden marker can sit right beside the caret, so the snap
decides where typing goes:
1. ✅ **Heading prefix → the caret lands AFTER the hidden `## `** (*"we have to push the caret to after the hidden ##"*).
   ⛔ Today's snap lands BEFORE a hidden run, which would make typing write `x## Heading` and destroy the heading.
2. ✅ **Opening inline marker → AFTER it** (*"If I position the cursor at the start of a bold word, my expectation is that
   typing will prepend the bold attributed section. So, that will need to 'push forward'."*).
3. ✅ **Closing inline marker → BEFORE it; no special handling** (*"If I position at the end of a bold word and start to
   type my expectation is that I am continuing the bold text. If I position at the space after the bold word my
   expectation is that I am typing normal text … no real problem exists"*). ⚠️ My SP-159 note that the writer "cannot
   type un-bold text between `bold` and the `.`" described exactly this behaviour; the user ruled it CORRECT.
4. ✅ **A formatted section always begins and ends with a VISIBLE character:** the commands strip leading/trailing
   whitespace when creating one (user: *"when creating an attributed section, we strip its whitespace"*). ✅ Already
   the E2 design §6.1 rule (CommonMark flanking needs it) → EP-046 **E2-S3** (AC6).

✅ **WHERE IT LIVES — recommended 2026-10-05, user concurs:** the **View menu** (a display toggle, like panel
visibility), persisted **inside the project** like panel visibility (`inspector-layout.json`, through ScriviCore). ✅ **RULED 2026-10-07 ([EP-047] P1):
`project-settings.json`**, not `inspector-layout.json`.
⛔ NOT `ProjectPreferences`: that class persists to `UserDefaults` (`ProjectPreferences.swift:3, 8, 67`), so it would
not travel with the project. ⚠️ Rule the exact file when this is scheduled.

3. Toggling it re-presents the visible text (an attributes-only edit, as the reveal does) — no rebuild, no history event.

✅ **ADDED 2026-10-05 (user, [SP-162] live pass):** *"That's ok. The bold level is good. I notice that hitting a nested italic inside a bold section also shows all the hints. I think I like it. I think the writer is going to want to turn it on and off a lot though. It allows the writer to verify where her formatting start and end (something MS Word does not do), and Markup editors like Ulysses show the markup characters all the time only smaller and hidden. Our goal is to allow the writer to forget that she is editing markup. When we implement the \"Show/Hide Markup Hints\" view option we should also investigate toggling it with a keystroke."*
4. ✅ **Investigate a KEYSTROKE toggle** alongside the View-menu item — the writer is expected to flip hints on and off
   often. ⚠️ Check the keyboard shortcuts already taken (menu bar + `ManuscriptNSTextView.keyDown`) before proposing one.
5. ✅ **The intent, in the user's words:** hints let the writer *"verify where her formatting start and end (something
   MS Word does not do)"*, while *"our goal is to allow the writer to forget that she is editing markup"* — so OFF must
   look like a finished page, and ON must cost nothing to reach.
6. ✅ **As built in E2-S2 ([SP-162]):** hints show at a section's FIRST/LAST character; a nested section reveals all its
   markers; a heading's `## ` shows on the caret's line.


🟠 **2026-10-08 — IMPLEMENTED - NOT VERIFIED** ([SP-170]): View ▸ Show Markup Hints (⇧⌘H), per project, default ON; caret rules 1–3 hold with hints off (tested); interop 243/243; 5/5 mutations killed. ⏳ Live pass → [`../Sprints/Sprint-SP-170.md`](../../Sprints/Closed/Sprint-SP-170.md).
✅ **VERIFIED 2026-10-08 (user, live pass):** *"1. passes. 2. passes. 3. passes. 4. passes. 5. passes."* ✅ Archived 2026-10-08 with the SP-170 close.
