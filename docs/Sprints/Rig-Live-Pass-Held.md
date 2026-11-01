# Held Rig Live Passes

Every Linux rig live pass that is waiting for the rig, in one place, so they can be run together when it is available.

- **One section per Sprint AC.** Each Sprint's AC links here; the Sprint file keeps the AC and its result.
- **When a section passes or fails,** record the result in its Sprint's progress log and **remove the section from this
  file** (this file holds what is still to run, not history).
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

## SP-195 AC6 — saves through the queue (EP-052 AC6) · ✅ Mac passed 2026-10-26

→ [`Sprint-SP-195.md`](Sprint-SP-195.md)
⚠️ Last attempt (2026-10-26) was blocked: the share could not be remounted with the project open (`umount`: *"target is
busy"*). Before step 4, run `fuser -vm /mnt/scrivi-net` on the rig and record what it names.

1. Type a few sentences into any scene, quickly. The text never waits or stutters.
2. **Edit ▸ Find and Replace…** (Ctrl+Alt+F). Find `the`, replace with `teh`, **Replace All**. The progress row counts
   (*"Replacing 312 of 2,048"*), the view follows, the window stays usable. Then **Edit ▸ Undo**. No freeze, and the
   text reads as before.
3. Type a sentence into one scene and remember it.
4. Pull the drive behind the share (or stop the share on the Mac). Type another sentence. Typing does not wait.
5. **File ▸ Close Project.** The dialog *"Some changes can't be saved"* appears. Choose **Cancel**: the window stays open.
6. Reconnect the drive and remount the share. Within a few seconds the held save lands (log: the queued save written).
7. Close the project and reopen it. Both sentences are there.

## SP-196 AC4 — structural edits through the queue · ✅ Mac passed 2026-10-27

→ [`Sprint-SP-196.md`](Sprint-SP-196.md) · Rig pass 2 (2026-10-27) got stuck after a split and its undo; the fix has not
been re-passed. Each step's undo returns the manuscript to where it was, so no step leaves a fork for the next.

1. Select from the middle of one scene to the middle of the next. **Edit ▸ Cut**, then **Edit ▸ Undo**, then
   **Edit ▸ Redo**, then **Edit ▸ Undo**. Each shows its cue at once and lands; the text reads as before.
2. Copy a paragraph (**Edit ▸ Copy**), click at the end of another scene, **Edit ▸ Paste**, then **Edit ▸ Undo**.
3. Select across a scene break and press **Backspace**, then **Edit ▸ Undo**.
4. Put the caret mid-scene, **Scene ▸ Split Scene** (Ctrl+Return), type a word straight away, then **Edit ▸ Undo** twice
   (the word, then the split). ⚠️ This is where pass 2 stuck: the window must stay usable, and the scene file must keep its
   meta beside it.
5. **Scene ▸ Merge Scene** (Ctrl+Backspace), then **Edit ▸ Undo**.
6. **Chapter ▸ Split Chapter**, then **Edit ▸ Undo**; **Chapter ▸ Merge Chapter**, then **Edit ▸ Undo**.
7. Press Ctrl+Z twice quickly after any one step: the second waits its turn and nothing is cancelled.
8. **File ▸ Quit.** It finishes without waiting forever; relaunch and the manuscript reads as before step 1.

## SP-197 AC5 — reads off the main thread (Inspector, timeline) · ✅ Mac passed 2026-11-01

→ [`Sprint-SP-197.md`](Sprint-SP-197.md) · Rig pass 1 (2026-10-27) failed: the Inspector's cards never enabled. Fix not
yet re-passed.

1. **View ▸ Show Inspector.** Scroll steadily through the manuscript. No freeze; the Inspector dims the old cards at once
   and shows *"Loading…"* only if a read takes over 400 ms.
2. Stop on a scene. Its cards fill and become editable. (A slow read shows *"Still reading this scene…"* at 5 s and still fills.)
3. Click scenes in the Navigator (**View ▸ Show Scene Navigator**) one after another, quickly. The cards end on the
   last scene clicked.
4. Note how long the cards take to fill on a scene (from the log). That time is SP-197's Q4 measurement.

## SP-198 / I-0326 — Replace All keeps headings (the Linux half)

→ [`../Issues/Issue-active.md`](../Issues/Issue-active.md) (I-0326)

1. **Edit ▸ Find and Replace…** (Ctrl+Alt+F). Untick **Ignore Case**. Find `The`, replace with `That`, **Replace All**.
2. Scroll through several chapters. Every heading that started "The" now starts "That" and is still a heading (the `##`
   is not lost).
3. **Edit ▸ Undo.** The headings read "The" again.

## SP-199 AC6 — T-0640, T-0641: Timeline zoom-to-scene and time scale · ✅ iPad and Mac passed 2026-11-01

→ [`Sprint-SP-199.md`](Sprint-SP-199.md)

1. **View ▸ Show Timeline** (Ctrl+Alt+M). A row of ticks runs along the top of the strip, labelled in years
   ("Year 1", "Year 2", …).
2. Click a scene in the manuscript, then click the `◎` button left of `+` and `−` at the bottom right. The strip zooms
   until the dots near that scene are separate, the scene's dot is centred and highlighted, and the labels change to
   days or hours ("Day 3", "Day 3 14:00").
3. Drag across an empty part of the strip (or use the scrollbar) to a stretch with no dots. The ticks and labels move
   with the drag.
4. **File ▸ Close Project**, reopen it from the landing window, then **File ▸ Quit** and relaunch. Both times the
   Timeline comes back at the same zoom and place.

---

*Last Updated: 2026-11-01 (SP-197 Mac pass recorded.)*

*Last Updated: 2026-11-01 (created: SP-195 AC6, SP-196 AC4, SP-197 AC5, I-0326, SP-199 AC6.)*
