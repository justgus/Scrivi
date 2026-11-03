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

✅ **None** (2026-11-02). [T-0648]–[T-0655] all passed on the Linux VM (`scrivi-vm`, builds 1.0.316–1.0.321; the user's
checklist: Pass) and are archived in `../Tasks/Verified/`. Their steps are in those archives.

---

*Last Updated: 2026-11-02 (all eight held passes run on the VM and passed; their sections removed.)*

*Last Updated: 2026-11-02 (T-0648: open from the SCRIVI-OTHE share; eject or pull, never stop the share — user.)*

*Last Updated: 2026-11-02 (became EP-057's step book; each section headed by its Task, T-0648–T-0655.)*

*Last Updated: 2026-11-01 (SP-201 AC6 added.)*

*Last Updated: 2026-11-01 (SP-200 AC6 added; Mac pass recorded.)*

*Last Updated: 2026-11-01 (SP-200 AC5 added.)*

*Last Updated: 2026-11-01 (SP-197 Mac pass recorded.)*

*Last Updated: 2026-11-01 (created: SP-195 AC6, SP-196 AC4, SP-197 AC5, I-0326, SP-199 AC6.)*
