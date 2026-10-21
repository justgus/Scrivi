# Verified Task — T-0609

**Sprint:** [SP-182] · **Epic:** [EP-048] · **Platform:** `[Linux]`
**Implemented:** 2026-10-15 · ✅ **USER-VERIFIED 2026-10-21** (user, 2026-10-21: *"I performed the live pass … on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*) — SP-182 AC5: rig live pass steps 5, 7, 14, 15 (typeface crossing both ways), 22–24; Q6 ruled at step 22 (synthesized bold acceptable).
**Archived:** 2026-10-21, on the user's report of the pass; the Sprint closed the same day (user-approved).

---

## T-0609 — `[Linux]` EP-048 S5a: Project Settings dialog, the bundled typefaces, Markup Hints

**Epic:** [EP-048] (S5a, L10) · **Status:** ✅ **Verified 2026-10-21** (user, 2026-10-21: *"I performed the live pass … on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*) · **Sprint:** [SP-182] →
[`../../Sprints/Closed/Sprint-SP-182.md`](../../Sprints/Closed/Sprint-SP-182.md) · **Date Requested:** 2026-10-15 (user: *"plan EP-048 fully."*)

**What:**
- Replace the T-0312 *"coming soon"* stub with Linux's Project Settings dialog (Project: Title, Subtitle · Typography:
  Typeface, Text size).
- Back it with a Linux `ProjectPreferences` in Apple's shape over the shared `project-settings.json`: the whole document
  kept, keys written only once chosen.
- Register the seven bundled faces from `Resources/Fonts/fonts.json` and ship their OFL texts. The manuscript, headings and
  chapter titles draw in the project's face and size.
- View ▸ Show Markup Hints (Ctrl+Shift+H).
- Variable-font weights are measured on Qt 6.4.2 and 6.10.2 first.

**Done (2026-10-15):** `ManuscriptTypography` (`BundledFonts` from `fonts.json`, compiled in), `ProjectPreferences`,
`ProjectSettingsDialog`, the presenter's hints switch and chapter titles, View ▸ Show Markup Hints. Proven by a new
`settings_smoke` (CI), 5 mutations caught.
