# Held Rig Live Passes

Every Linux rig live pass that is waiting for the rig, in one place, so they can be run together when it is available.

- **The step book of [EP-057] Linux Rig Verification** (rule: `Sprint-GUIDELINES.md`, *"The rig pass never holds a
  Sprint open"*). **One section per EP-057 Task**, headed by its Task ID; the Sprint it came from is named after it.
- **When a section passes or fails,** record the result in **its Task** (a failure is a NEW Issue) and **remove the
  section from this file** (this file holds what is still to run, not history). The Sprint it came from may be closed.
- **New held steps go here,** never only in a Sprint file or only in the chat.

## Before any section

1. On the Mac: back up `~/ScriviLinux/projects/dumas-prose-timelines` (copy the folder).
2. Mount the shares on the rig from the Mac: `./scripts/mount-shares-on-rig.sh`.
3. On the rig: `git pull` and build ([`../Scrivi_Linux_Rig_Setup_v0_1.md`](../Scrivi_Linux_Rig_Setup_v0_1.md)). Launch Scrivi
   and check **Help ▸ About Scrivi** shows the build you just pulled.
4. Open **dumas-prose-timelines over the share** (File ▸ Open Project…, under `/mnt/scrivi-net/projects`).
5. Keep **Help ▸ Show Log Folder** open; the times for each step are in that log.

⚠️ Use the menus, not Ctrl+Shift shortcuts, if the session drops Shift combinations.

---

## Held now

### T-0660 — Orca reads the manuscript as the page shows it ([SP-203], EP-050 S4; [SP-202] Q3)

**Where:** scrivi-vm (counts: no x86, GPU or real network). **Build:** 1.0.328 (`1be7b83`) or later — Help ▸ About Scrivi.
No share needed; a new local project.

1. Launch Scrivi from the desktop launcher. **Help ▸ About Scrivi** shows 1.0.328 or later. Close the About box.
2. **File ▸ New Project…** — the New Project dialog: **Title:** `Orca test`; keep the suggested **Location:**; **Create**.
3. In the manuscript, type: `Mr. Smith said ` — then **Format ▸ Bold**, type `bold`, **Format ▸ Bold** again — type ` and ` —
   **Format ▸ Italic**, type `soft`, **Format ▸ Italic** — type ` here.` and press **Return** twice.
4. **Format ▸ Heading 2**, type `The claim stated plainly`, press **Return** twice, type `1. not a list.`
5. Turn **Orca on**: Super+Alt+S (Orca says "Screen reader on"). Click back into the manuscript.
6. Press **Up** and **Down** over the lines. ✅ Orca reads *"Mr. Smith said bold and soft here."* — never "star", "backslash"
   or "number sign"; *"The claim stated plainly"* — no "number number"; *"1. not a list."* with no "backslash".
   Record what Orca says for the Mr. line exactly (Orca's default punctuation level).
7. On the Mr. line, **Right** / **Left** one character at a time through *"bold"*. ✅ Orca speaks each letter; never a "star",
   and the caret never stops twice in one place.
8. Put the caret at the start of `1. not a list.` and choose **Scene ▸ Split Scene** (a divider appears above it). Press
   **Up** from that line. ✅ Orca reads *"Scene break"* on the divider line.
9. Type a few words with Orca on. ✅ It echoes what you type; nothing freezes.
10. Optional: open **dumas-prose-timelines** with Orca on. Note whether the window pauses for a moment when Orca first reads
    the manuscript (measured: about 0.2 s once).
11. Turn **Orca off**: Super+Alt+S. Quit Scrivi (**File ▸ Quit**). The `Orca test` project can be deleted afterwards.

**Pass:** 6–9 as marked. A failure is a new Issue, linked from [T-0660].

---

*Last Updated: 2026-11-04 (T-0660 added: the Orca listen, EP-050 S4.)*

*Last Updated: 2026-11-02 (all eight held passes run on the VM and passed; their sections removed.)*

*Last Updated: 2026-11-02 (T-0648: open from the SCRIVI-OTHE share; eject or pull, never stop the share — user.)*

*Last Updated: 2026-11-02 (became EP-057's step book; each section headed by its Task, T-0648–T-0655.)*

*Last Updated: 2026-11-01 (SP-201 AC6 added.)*

*Last Updated: 2026-11-01 (SP-200 AC6 added; Mac pass recorded.)*

*Last Updated: 2026-11-01 (SP-200 AC5 added.)*

*Last Updated: 2026-11-01 (SP-197 Mac pass recorded.)*

*Last Updated: 2026-11-01 (created: SP-195 AC6, SP-196 AC4, SP-197 AC5, I-0326, SP-199 AC6.)*
