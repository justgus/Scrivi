# T-0359 – T-0364 — EP-030 Card Framework & Writing-Tool Cards (SP-090, SP-091)

**Status:** ✅ **All Verified 2026-08-05**
**Epic:** EP-030 — Scene Inspector Card Framework · ✅ **Closed** →
[`Epic-EP-030.md`](../../Epics/Closed/Epic-EP-030.md)
**Records of truth:** [`Sprint-SP-090.md`](../../Sprints/Closed/Sprint-SP-090.md) ·
[`Sprint-SP-091.md`](../../Sprints/Closed/Sprint-SP-091.md)
**Design:** `docs/Scrivi_Scene_Inspector_Card_Framework_v0_1.md` ✅ Approved 2026-08-05
(C1=C · C2=B · C3=A within tabs · C4=A · C5=C + in-stack creation · C6=B with **sort per-stack**)

---

## SP-090 — Framework

**T-0359 — `InspectorCard` protocol + registry + `CardContext`.** ✅ Verified 2026-08-05 (High).
Per §5: `typeID` (stable, persisted), `title`, an associated `Body`, and `CardContext`
(`sceneID`, `engine`, `config`). A registry maps `typeID` → implementation so the layout file stays
decoupled from Swift types. Swift is UI only — cards reach ScriviCore through `ScriviEngine`.

**T-0360 — `inspector-layout.json` schema + load/save + unknown-`typeID` skip.** ✅ Verified
2026-08-05 (High). Schema `scrivi.inspector-layout.v1` (§C2): `selectedTab`, `inspectorHidden`,
`defaultStacks` (worldbuilding **empty**, writing = `tags`/`outline`/`todo`), `stackSort`, and
per-scene `worldbuilding`/`writing` arrays. Project-level and Git-visible — **not** the scene sidecar
(EP-027 keeps those canonical) and **not** workspace state, because a card stack is a creative
decision that should travel with the project. An unknown `typeID` is **skipped with a notice, never a
crash**.

**T-0361 — Bottom tabs + persisted `selectedTab` + menu hide/show.** ✅ Verified 2026-08-05 (High).
Order `Writing | Worldbuilding | Properties`, defaulting to **Writing**; tab selection **does not
follow the scene**. `selectedTab` and `inspectorHidden` persist at project level — the latter
promoting the former in-memory `session.inspectorVisible` to persisted state. Replaced the fixed
280 pt frame with a resizable pane, and replaced `SceneInspectorView`.

**T-0362 — Card stack mechanics.** ✅ Verified 2026-08-05 (Medium). Add / remove / reorder /
collapse; **sort is per-stack, not per-card** (C6); "apply to all scenes" and "to this chapter" (C1),
with each scene independently editable afterward.

## SP-091 — Writing-tool cards

**T-0363 — `tags` + `todo` cards.** ✅ Verified 2026-08-05 (High). Scene-sidecar backed, on the
`scrivi.scene.v1` extensions shipped alongside by T-0392/T-0393.

**T-0364 — `outline` card.** ✅ Verified 2026-08-05 (High). Scene summary / synopsis.

> **T-0365 (`sources` card + `source` object kind) was deferred out of SP-091** (ruled 2026-08-05)
> and split across three sprints. Its ScriviCore halves shipped as the `cites`/`documented-by`
> relation type (SP-096/T-0373) and the `source` object kind (SP-098/T-0406), both ✅ Verified
> 2026-08-12. The remaining aggregate `sources` card is 🟡 **active in SP-102** — see
> [`Task-active.md`](../Task-active.md).

---

*Archived 2026-08-18 during the tracking-documentation audit. These rows had remained in
`Task-backlog.md` after SP-090 and SP-091 closed; the Sprint archives were and remain the
authoritative records.*
