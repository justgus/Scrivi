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

### [Mac] T-0618 / T-0616 — the same, on the Mac, on a LOCAL disk ([SP-190] AC2, [SP-189] AC5)

**Held here with the rig's at the user's instruction (2026-11-04: *"Put all the live pass steps in the EP-057 Live pass"*)** —
it is not an EP-057 Task: its result goes in [T-0618] (and [T-0616]). ✅ **Re-ruled 2026-11-04 (user): on a local disk** — the
two-hop Samba loop was too slow to be useful; the share half is Linux's ([T-0661]).

**Build:** the Mac app from `main` at or after `c4799ac` — Scrivi ▸ About Scrivi. Copy dumas to a local folder (keep the original
untouched), open the copy (File ▸ Open…), then **Help ▸ Show Log Folder** (Finder shows `scrivi.log`).

The steps (as [T-0661] ran them on the rig, ✅ passed 2026-11-04 and archived):

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
8. Note the word the caret is on; **Quit**, launch again, open the same copy: ✅ the window shows that word, the caret on it.
9. **Help ▸ Show Log Folder**: copy `scrivi.log` (and `scrivi.log.1`, the session before the reopen) somewhere safe.


On the Mac, with the Mac's commands: **Scene ▸ New Scene (⌘↩)** for a split, **Scene ▸ Merge Scene with
Previous (⌘⌫)** for a merge, **Chapter ▸ New Chapter (⇧⌘↩)** for a chapter split, **Chapter ▸ Merge Chapter with Previous
(⇧⌘⌫)**; **Edit ▸ Copy / Cut / Paste**; Delete (⌫) over a selection; **Edit ▸ Undo (⌘Z) / Redo (⇧⌘Z)** — ⚠️ never a new edit
between an Undo and its Redo, or the Mac's branch chooser opens on ⇧⌘Z. Quit with **Scrivi ▸ Quit Scrivi**. The same ✅ marks;
the log lines are the same words (`structural: … landed after N ms in all`, `verify: …`, `⚠ the window is not answering`).

---

*Last Updated: 2026-11-04 (T-0661 passed on the VM (run 2) and is archived; its steps kept in the Mac section.)*

*Last Updated: 2026-11-04 (T-0660 passed on the VM and is archived; its section removed.)*

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
