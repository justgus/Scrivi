# Live Pass Checklist — EP-057 held passes — 2026-11-02

Evidence record for the held Linux live passes ([`Rig-Live-Pass-Held.md`](Rig-Live-Pass-Held.md), copied 2026-11-02).
Tick each step as it passes; record each Task's result in its block, then in the Task itself.

| | |
|---|---|
| **Machine** | ☐ scrivi-vm (VM, counts per Sprint-GUIDELINES rule 6) ☐ Oathkeeper (rig) |
| **Build** (Help ▸ About) | `Scrivi (Linux) 1.0.___ (_______)` — expected 1.0.314 (`225470d`) or later |
| **Date / tester** | |
| **SMB delay** (VM) | ☐ `scrivi-smb-delay` active (5 ms one way) |
| **Backup taken** | ☐ `~/ScriviLinux/projects/dumas-prose-timelines` copied |

---

## Before any section

- [ ] 1. On the Mac: back up `~/ScriviLinux/projects/dumas-prose-timelines` (copy the folder).
- [ ] 2. Mount the shares on the rig from the Mac: `./scripts/mount-shares-on-rig.sh` (VM: `SCRIVI_RIG=scrivi-vm ./scripts/mount-shares-on-rig.sh`).
- [ ] 3. On the rig: `git pull` and build ([`../Scrivi_Linux_Rig_Setup_v0_1.md`](../Scrivi_Linux_Rig_Setup_v0_1.md)). Launch Scrivi
  and check **Help ▸ About Scrivi** shows the build you just pulled.
- [ ] 4. Open **dumas-prose-timelines over the share** (File ▸ Open Project…, under `/mnt/scrivi-net/projects`).
- [ ] 5. Keep **Help ▸ Show Log Folder** open; the times for each step are in that log.

⚠️ Use the menus, not Ctrl+Shift shortcuts, if the session drops Shift combinations.

**Before any section — done:** ☐

---

## T-0648 · SP-195 AC6 — saves through the queue (EP-052 AC6) · ✅ Mac passed 2026-10-26

→ [`Sprint-SP-195.md`](Sprint-SP-195.md)
⚠️ Last attempt (2026-10-26) was blocked: the share could not be remounted with the project open (`umount`: *"target is
busy"*). Before step 4, run `fuser -vm /mnt/scrivi-net` on the rig and record what it names.
⚠️ **Open dumas-prose-timelines from `/mnt/scrivi-other`** (the SCRIVI-OTHE drive), not `/mnt/scrivi-net` (the Mac's own disk).
Eject or pull the drive; never stop the share. Step 5 needs a build with the [I-0335] fix.

- [ ] 1. Type a few sentences into any scene, quickly. The text never waits or stutters.
- [ ] 2. **Edit ▸ Find and Replace…** (Ctrl+Alt+F). Find `the`, replace with `teh`, **Replace All**. The progress row counts
  (*"Replacing 312 of 2,048"*), the view follows, the window stays usable. Then **Edit ▸ Undo**. No freeze, and the
  text reads as before.
- [ ] 3. Type a sentence into one scene and remember it.
- [ ] 4. Eject or pull the SCRIVI-OTHE drive on the Mac. Type another sentence. Typing does not wait.
- [ ] 5. **File ▸ Close Project.** The dialog *"Some changes can't be saved"* appears. Choose **Cancel**: the window stays open.
- [ ] 6. Reconnect the drive, then remount the share (VM: `SCRIVI_RIG=scrivi-vm ./scripts/mount-shares-on-rig.sh --only ScriviOther`). Within a few seconds the held save lands (log: the queued save written).
- [ ] 7. Close the project and reopen it. Both sentences are there.

**T-0648 result:** ☐ Pass ☐ Fail — notes / log times / new Issue:

> 

---

## T-0649 · SP-196 AC4 — structural edits through the queue · ✅ Mac passed 2026-10-27

→ [`Sprint-SP-196.md`](Sprint-SP-196.md) · Rig pass 2 (2026-10-27) got stuck after a split and its undo; the fix has not
been re-passed. Each step's undo returns the manuscript to where it was, so no step leaves a fork for the next.

- [ ] 1. Select from the middle of one scene to the middle of the next. **Edit ▸ Cut**, then **Edit ▸ Undo**, then
  **Edit ▸ Redo**, then **Edit ▸ Undo**. Each shows its cue at once and lands; the text reads as before.
- [ ] 2. Copy a paragraph (**Edit ▸ Copy**), click at the end of another scene, **Edit ▸ Paste**, then **Edit ▸ Undo**.
- [ ] 3. Select across a scene break and press **Backspace**, then **Edit ▸ Undo**.
- [ ] 4. Put the caret mid-scene, **Scene ▸ Split Scene** (Ctrl+Return), type a word straight away, then **Edit ▸ Undo** twice
  (the word, then the split). ⚠️ This is where pass 2 stuck: the window must stay usable, and the scene file must keep its
  meta beside it.
- [ ] 5. **Scene ▸ Merge Scene** (Ctrl+Backspace), then **Edit ▸ Undo**.
- [ ] 6. **Chapter ▸ Split Chapter**, then **Edit ▸ Undo**; **Chapter ▸ Merge Chapter**, then **Edit ▸ Undo**.
- [ ] 7. Press Ctrl+Z twice quickly after any one step: the second waits its turn and nothing is cancelled.
- [ ] 8. **File ▸ Quit.** It finishes without waiting forever; relaunch and the manuscript reads as before step 1.

**T-0649 result:** ☐ Pass ☐ Fail — notes / log times / new Issue:

> 

---

## T-0650 · SP-197 AC5 — reads off the main thread (Inspector, timeline) · ✅ Mac passed 2026-11-01

→ [`Sprint-SP-197.md`](Sprint-SP-197.md) · Rig pass 1 (2026-10-27) failed: the Inspector's cards never enabled. Fix not
yet re-passed.

- [ ] 1. **View ▸ Show Inspector.** Scroll steadily through the manuscript. No freeze; the Inspector dims the old cards at once
  and shows *"Loading…"* only if a read takes over 400 ms.
- [ ] 2. Stop on a scene. Its cards fill and become editable. (A slow read shows *"Still reading this scene…"* at 5 s and still fills.)
- [ ] 3. Click scenes in the Navigator (**View ▸ Show Scene Navigator**) one after another, quickly. The cards end on the
  last scene clicked.
- [ ] 4. Note how long the cards take to fill on a scene (from the log). That time is SP-197's Q4 measurement.

**T-0650 result:** ☐ Pass ☐ Fail — notes / log times / new Issue:

> 

---

## T-0651 · SP-198 / I-0326 — Replace All keeps headings (the Linux half)

→ [`../Issues/Issue-active.md`](../Issues/Issue-active.md) (I-0326)

- [ ] 1. **Edit ▸ Find and Replace…** (Ctrl+Alt+F). Untick **Ignore Case**. Find `The`, replace with `That`, **Replace All**.
- [ ] 2. Scroll through several chapters. Every heading that started "The" now starts "That" and is still a heading (the `##`
  is not lost).
- [ ] 3. **Edit ▸ Undo.** The headings read "The" again.

**T-0651 result:** ☐ Pass ☐ Fail — notes / log times / new Issue:

> 

---

## T-0652 · SP-199 AC6 — T-0640, T-0641: Timeline zoom-to-scene and time scale · ✅ iPad and Mac passed 2026-11-01

→ [`Sprint-SP-199.md`](Sprint-SP-199.md)

- [ ] 1. **View ▸ Show Timeline** (Ctrl+Alt+M). A row of ticks runs along the top of the strip, labelled in years
  ("Year 1", "Year 2", …).
- [ ] 2. Click a scene in the manuscript, then click the `◎` button left of `+` and `−` at the bottom right. The strip zooms
  until the dots near that scene are separate, the scene's dot is centred and highlighted, and the labels change to
  days or hours ("Day 3", "Day 3 14:00").
- [ ] 3. Drag across an empty part of the strip (or use the scrollbar) to a stretch with no dots. The ticks and labels move
  with the drag.
- [ ] 4. **File ▸ Close Project**, reopen it from the landing window, then **File ▸ Quit** and relaunch. Both times the
  Timeline comes back at the same zoom and place.

**T-0652 result:** ☐ Pass ☐ Fail — notes / log times / new Issue:

> 

---

## T-0653 · SP-200 AC5 — the history opens fast and reads back the same · ✅ Mac passed 2026-11-01

→ [`Sprint-SP-200.md`](Sprint-SP-200.md). The first open of each project compacts its history (`history/log-000002.jsonl`
appears beside `log-000001.jsonl`, which the next open removes).

- [ ] 1. Open **dumas-prose-timelines** over the share. Note how long until the manuscript can be typed into.
- [ ] 2. Type a sentence, then **Edit ▸ Undo** and **Edit ▸ Redo**: the sentence goes and comes back.
- [ ] 3. Select across two scenes and **Edit ▸ Cut**, then **Edit ▸ Undo**: both scenes read as before.
- [ ] 4. **Edit ▸ Find and Replace…**: replace `the` with `thee`, **Replace All**, then **Edit ▸ Undo**: every `the` is back.
- [ ] 5. **File ▸ Close Project**, reopen it, and **Edit ▸ Undo** twice more: the steps before the reopen undo as they did.
- [ ] 6. Repeat 1–5 on **dumas-prose-timelines-history** (25,000 history steps) — it should open as quickly.

**T-0653 result:** ☐ Pass ☐ Fail — notes / log times / new Issue:

> 

---

## T-0654 · SP-200 AC6 — T-0647: the Properties tab's metrics · ✅ Mac passed 2026-11-01

→ [`Sprint-SP-200.md`](Sprint-SP-200.md)

- [ ] 1. With dumas-prose-timelines open, **View ▸ Show Inspector**, then the **Properties** tab. Click a scene that has
  objects linked in its Worldbuilding tab.
- [ ] 2. The Scene section shows **Links** with the number of objects listed in that scene's Worldbuilding tab.
- [ ] 3. Below it, **Project**: Scenes, Chapters, Links, World links, and History ("N steps · N MB"). Scenes reads 1,175 or so.
- [ ] 4. Then one **World — <name>** section per world the project uses (none if it uses no world): Objects, then a row per
  kind (Character, Location, …).
- [ ] 5. Click another scene: the Scene numbers change, the Project and World numbers stay. The window never waits.
- [ ] 6. If a world is on a drive you can pull: pull it and click another scene. Its section reads **"(unavailable; last
  known)"** and still shows the counts.

**T-0654 result:** ☐ Pass ☐ Fail — notes / log times / new Issue:

> 

---

## T-0655 · SP-201 AC6 — I-0333, I-0334: Replace All without holding the window · ✅ Mac passed 2026-11-01

→ [`Sprint-SP-201.md`](Sprint-SP-201.md)

- [ ] 1. Open **dumas-prose-timelines** over the share. **Edit ▸ Find and Replace…** (Ctrl+Alt+F): find `the`, replace with
  `thee`, **Replace All**. The row counts (*"Replacing 12,000 of 41,296"*), the window stays usable, and it ends with every
  `the` replaced. Note the time from the log (`replace all: worker finished in … ms`).
- [ ] 2. **Edit ▸ Undo** once: every `thee` is `the` again.
- [ ] 3. While the saves are still being written (the row says so), hide and show the Inspector (**View ▸ Show Inspector**,
  Ctrl+Alt+I) twice. The window never waits.
- [ ] 4. **File ▸ Close Project**, reopen it: the Inspector is shown or hidden as you left it, and the text reads as after step 2.

**T-0655 result:** ☐ Pass ☐ Fail — notes / log times / new Issue:

> 

---

**Summary:** ☐ all Tasks passed ☐ failures filed as new Issues: ________
