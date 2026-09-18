# Verified Issues: I-0211 – I-0220

| ID | Issue | Priority | Sprint | Verified |
| -- | ----- | -------- | ------ | -------- |
| **I-0214** | `[ScriviCore]` ⚠️ **An imported timeline that FAILS TO PARSE is discarded in SILENCE.** ✅ **FOUND BY THE USER 2026-09-15**, ⚠️ **who rejected the first diagnosis on evidence: the timelines HAD displayed on the previous run.** ✅ **PROVEN: same 4 files — old Swift decoder → 4 rows/157 events; core parser → 0 rows.** ⚠️ **T-0502's regression, and the core reported nothing.** | **High** | SP-129 | 2026-09-15 |
| **I-0215** | `[Apple]` ⛔ **Apple SILENTLY DISCARDED any `inspector-layout.json` key it did not know, on every save** — ⚠️ **`InspectorLayoutDocument` was a fixed 6-property `Codable` struct, so a round trip through Apple DROPPED unknown keys, while LINUX deliberately preserved them.** ⚠️ **The user opens the same projects on both.** ✅ **FIXED by retaining the raw document alongside the typed view and merging on save** (T-0536). ⚠️ **TOP-LEVEL keys only — nested-in-card keys remain lost, deliberately deferred to [SP-141].** ⛔ **Proven by a test demonstrated to FAIL without the fix.** | **High** | SP-140 | 2026-09-18 |
