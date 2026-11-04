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

### T-0661 — EP-053 over the share: every structural edit timed, verified, no freeze ([SP-189] AC5, [SP-190] AC3)

**Where:** scrivi-vm (its shares carry the 5 ms SMB delay; counts for this — no x86, GPU or real network). **Build:** 1.0.330
(`c4799ac`) or later — Help ▸ About Scrivi. Do **Before any section** first (back up dumas, mount, pull + build, open
**dumas-prose-timelines over the share**, Help ▸ Show Log Folder). ⚠️ Each step ends with the edit APPLIED (do it, Undo, Redo),
so no step's undo leaves a fork for the next. No dialog appears in any step.

For EVERY step: ✅ the window never stops answering (a short read-only moment is fine); ✅ the manuscript reads as the same
edit made by hand would; ✅ the log has `structural "…": applied in N ms, M ms in all` and then
`verify: … 0 corrected, 0 skipped (typed since), 0 not correctable`; ⛔ no `the window is not answering` line. Note each
`applied … in all` time.

1. **Split Scene:** caret in the middle of a paragraph in chapter 2 → **Scene ▸ Split Scene** (a divider at the caret).
   **Edit ▸ Undo** (gone), **Edit ▸ Redo** (back).
2. **Merge Scene:** caret at the very start of the scene step 1 made → **Scene ▸ Merge Scene** (the divider goes). Undo, Redo.
3. **Split Chapter:** caret in the middle of a scene in chapter 3 → **Chapter ▸ Split Chapter** (a new chapter heading at the
   caret; the chapters after it renumber). Undo, Redo.
4. **Merge Chapter:** caret at the start of the first scene of the chapter step 3 made → **Chapter ▸ Merge Chapter**. Undo, Redo.
5. **Copy and paste across scenes:** select from the middle of one scene to the middle of the next (across a divider) →
   **Edit ▸ Copy**; caret in the middle of a paragraph in chapter 5 → **Edit ▸ Paste** (it lands with its divider). Undo, Redo.
6. **Cut across scenes:** select across a divider in chapter 6 → **Edit ▸ Cut** (the two scenes join where the cut was). Undo, Redo.
7. **Backspace over scenes:** select across a divider in chapter 7 → press **Backspace** (as Cut, without the clipboard). Undo, Redo.
8. **File ▸ Quit**, launch again, open dumas over the share: ✅ the manuscript reads as you left it after step 7.
9. Copy the log (Help ▸ Show Log Folder → `scrivi.log`) to `~/evidence-2026-11-xx/` before running anything else.

**Pass:** every step as marked. A failure is a new Issue, linked from [T-0661]. Afterwards restore dumas from the backup.

---

### [Mac] T-0618 / T-0616 — the same, on the Mac over a share ([SP-190] AC2, [SP-189] AC5; [SP-188] AC11's carried pass)

**Held here with the rig's at the user's instruction (2026-11-04: *"Put all the live pass steps in the EP-057 Live pass"*)** —
it is not an EP-057 Task: its result goes in [T-0618] (and [T-0616]). ✅ **The share ([SP-190] Q2): scrivi-vm's Samba loop** —
the Mac's own drive back through two hops (Mac → VM's CIFS mount → VM's Samba → Mac).

**Once, first:** on the VM, `sudo smbpasswd -a justgus` (choose the password). On the Mac, Finder ▸ Go ▸ Connect to Server…
(⌘K) → `smb://192.168.64.2/ScriviOtherLoop` → Connect, as `justgus` with that password; again for `ScriviWorldsLoop` (dumas's
world). Make a copy of dumas on the drive for this pass (as [SP-188]'s `dumas-loop-test.scrivi`) and open THAT copy from the
`ScriviOtherLoop` volume.

**Build:** the Mac app from `main` at or after `c4799ac` — Scrivi ▸ About Scrivi. Back up the dumas copy on the share first.
Open it from the loop volume (File ▸ Open…), then **Help ▸ Show Log Folder** (Finder shows `scrivi.log`).

Steps 1–8 as the rig's above, with the Mac's commands: **Scene ▸ New Scene (⌘↩)** for a split, **Scene ▸ Merge Scene with
Previous (⌘⌫)** for a merge, **Chapter ▸ New Chapter (⇧⌘↩)** for a chapter split, **Chapter ▸ Merge Chapter with Previous
(⇧⌘⌫)**; **Edit ▸ Copy / Cut / Paste**; Delete (⌫) over a selection; **Edit ▸ Undo (⌘Z) / Redo (⇧⌘Z)** — ⚠️ never a new edit
between an Undo and its Redo, or the Mac's branch chooser opens on ⇧⌘Z. Quit with **Scrivi ▸ Quit Scrivi**. The same ✅ marks;
the log lines are the same words (`structural: … landed after N ms in all`, `verify: …`, `⚠ the window is not answering`).

---

### T-0660 — Orca reads the manuscript as the page shows it ([SP-203], EP-050 S4; [SP-202] Q3)

**Where:** scrivi-vm (counts: no x86, GPU or real network). **Build:** 1.0.328 (`1be7b83`) or later — Help ▸ About Scrivi.
No share needed; a new local project.

1. Launch Scrivi from the desktop launcher. **Help ▸ About Scrivi** shows 1.0.328 or later. Close the About box.
2. **File ▸ New Project…** — the New Project dialog: **Title:** `Orca test`; keep the suggested **Location:**; **Create**.
3. In the manuscript, type: `Mr. Smith said ` — then **Format ▸ Bold**, type `bold`, **Format ▸ Bold** again — type ` and ` —
   **Format ▸ Italic**, type `soft`, **Format ▸ Italic** — type ` here.` and press **Return** twice.
4. **Format ▸ Heading 2**, type `The claim stated plainly`, press **Return** twice, type `1. not a list.`
5. Turn **Orca on WITH ITS SPEECH LOG** (2026-11-04: the VM's sound stutters, so what Orca says is read from its log, not
   heard): open a Terminal in the VM and type `orca -r --debug-file=/tmp/orca.log &` then Return. Click back into the
   manuscript. (Settings ▸ Accessibility ▸ Seeing ▸ Screen Reader only turns Orca on — without the log.)
6. Press **Up** and **Down** over the lines. ✅ Orca reads *"Mr. Smith said bold and soft here."* — never "star", "backslash"
   or "number sign"; *"The claim stated plainly"* — no "number number"; *"1. not a list."* with no "backslash".
   (What Orca said is read afterwards from `/tmp/orca.log`: each utterance is a `SPEECH OUTPUT: '…'` line.)
7. On the Mr. line, **Right** / **Left** one character at a time through *"bold"*. ✅ Orca speaks each letter; never a "star",
   and the caret never stops twice in one place.
8. Put the caret at the start of `1. not a list.` and choose **Scene ▸ Split Scene** (a divider appears above it). Press
   **Up** from that line. ✅ Orca reads *"Scene break"* on the divider line.
9. Type a few words with Orca on. ✅ It echoes what you type; nothing freezes.
10. Optional: open **dumas-prose-timelines** with Orca on. Note whether the window pauses for a moment when Orca first reads
    the manuscript (measured: about 0.2 s once).
11. Turn **Orca off**: in the Terminal, `pkill orca` (the screen reader setting starts the usual one again later). Tell Claude
    the log is there: it reads `/tmp/orca.log` over SSH. Quit Scrivi (**File ▸ Quit**). The `Orca test` project can be deleted afterwards.

**Pass:** 6–9 as marked. A failure is a new Issue, linked from [T-0660].

---

*Last Updated: 2026-11-04 (T-0661 and the Mac section added: EP-053 S4–S5, user: "Put all the live pass steps in the EP-057 Live pass".)*

*Last Updated: 2026-11-04 (T-0660 added: the Orca listen, EP-050 S4.)*

*Last Updated: 2026-11-02 (all eight held passes run on the VM and passed; their sections removed.)*

*Last Updated: 2026-11-02 (T-0648: open from the SCRIVI-OTHE share; eject or pull, never stop the share — user.)*

*Last Updated: 2026-11-02 (became EP-057's step book; each section headed by its Task, T-0648–T-0655.)*

*Last Updated: 2026-11-01 (SP-201 AC6 added.)*

*Last Updated: 2026-11-01 (SP-200 AC6 added; Mac pass recorded.)*

*Last Updated: 2026-11-01 (SP-200 AC5 added.)*

*Last Updated: 2026-11-01 (SP-197 Mac pass recorded.)*

*Last Updated: 2026-11-01 (created: SP-195 AC6, SP-196 AC4, SP-197 AC5, I-0326, SP-199 AC6.)*
