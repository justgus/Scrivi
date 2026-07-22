# Active Epics

_(No Active Epic.)_

**EP-028** `[Cross]` (Scene & Chapter Merging — Linux Parity & Filesystem-Coherence Fix) ✅ **closed
2026-07-22 (Human-approved)** — cross-platform keyboard scene/chapter merge (macOS + Linux) backed by two
new atomic ScriviCore endpoints, fixing the I-0083 chapter-merge data-loss regression. 4 sprints
(SP-074 `[ScriviCore]`, SP-075 `[Apple]`, SP-076 `[Linux]`, SP-077 `[Linux]` menu bar), AC1–AC7 all Verified.
Archived to `Closed/Epic-EP-028.md`.

**Next in line:** EP-024–EP-026 `[Linux]` (🔵 Draft — Scene Inspector Panel, Timeline Panel, Undo/Redo &
Menus & Settings & Parity Verification), promoted to Active one at a time on user request. **EP-019** `[Apple]`
(Custom Undo/Redo History & Multiple Copy Buffers) remains 🔴 Deferred (backlog); AC1/AC3/AC4/AC5 verified,
resumes at SP-056.

See: [Epic-backlog.md](Epic-backlog.md)

---

*Last Updated: 2026-07-22 (**EP-028 `[Cross]` ✅ closed with user approval** — delivered scene- and
chapter-merge by keyboard on both macOS and Linux, backed by SP-074's atomic `scrivi_merge_scene` /
`scrivi_merge_chapter` endpoints, and fixed the EP-027 chapter-merge data-loss regression (I-0083) at the
core. SP-076 brought Linux to parity; SP-077 added the native menu bar that made chapter-merge verifiable
over VNC (macOS→VNC swallows `Ctrl-Shift-Backspace`) and closed a data-loss hole in the editor-leaving paths.
AC1–AC7 all Verified; I-0083/I-0084 Verified, I-0085 Not-a-Bug, I-0086 fixed. Archived to
`Closed/Epic-EP-028.md`. Active Epics 1→0. Next in line: EP-024–EP-026 `[Linux]` (Draft).)*
