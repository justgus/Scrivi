# Scrivi — Timeline Dot Clustering & Zoom Refinement Design v0.1

**Status:** 🟡 **DRAFT — for ruling.** ⚠️ **NOT approved.**
**Date:** 2026-09-14
**Codebase:** `[Apple]` — ⚠️ **`Scrivi/Views/TimelineStripView.swift`.** ✅ **No ScriviCore change.**
**Supersedes nothing.** ✅ **Extends [`Scrivi_Timeline_Panel_Design_v0_3.md`](Scrivi_Timeline_Panel_Design_v0_3.md)
— ⚠️ that doc specifies clustering EXISTS; ✅ this one specifies WHAT IT RESOLVES TO.**
**Occasioned by:** ⚠️ **[I-0208]** + ✅ **the user's design ruling 2026-09-14.**

---

## 1. ✅ The user's rule, stated as spec

⚠️ **VERBATIM (2026-09-14):**
> ⚠️ *"what it should have done was cluster the dots so that a maximum number of aggregate dots could
> be presented across the width of the Timeline View… as I zoomed in… it should have continued to fill
> the viewport with ever decreasing aggregate dots… until only single dots remained… The timeline,
> when all 115x dots are aggregated into one dot, spans one dot. This is too narrow a span… the story
> timeline stretches from the first scene to the last scene and the minimum width of the timeline's
> scrolling region must be the width of the window."*

✅ **AS THREE BINDING RULES:**

| # | Rule |
| - | ---- |
| **R1** | ✅ **THE AXIS ALWAYS SPANS THE VIEWPORT.** ⚠️ **The story runs first-scene → last-scene, and that span NEVER renders narrower than the window.** |
| **R2** | ✅ **CLUSTER COUNT IS THE CONTROLLED VARIABLE.** ⚠️ **Aggregation resolves to as MANY distinct aggregates as FIT across the width** — ⛔ **never to "however many proximity chaining happens to yield".** |
| **R3** | ✅ **ZOOM REFINES MONOTONICALLY.** ⚠️ **Zooming in ⇒ more, smaller aggregates, until singles remain. Zooming out ⇒ fewer, larger.** ⛔ **No zoom step may INCREASE the member count of the cluster under the cursor.** |

---

## 2. ⚠️ What the code does today, and what is actually broken

### 2.1 ✅ R1 IS ALREADY SATISFIED — ⚠️ and this narrows the defect considerably

✅ **READ — the timeline is NOT a scrolling content region.** ✅ **It is a VIEWPORT-MAPPED PROJECTION:**

```swift
private func eventX(offsetMs: Int64, usable: CGFloat, panelW: CGFloat) -> CGFloat {
    let vSpan = visibleSpanMs()            // model.spanMs / zoom
    let vMin  = visibleMinMs()
    let fraction = CGFloat(offsetMs - vMin) / CGFloat(vSpan)
    return 16 + fraction * usable          // usable = panelW - 32
}
```

⚠️ **At `zoom = 1`, `visibleSpanMs() == model.spanMs`** — ✅ **so the first scene sits at `x = 16` and
the last at `x = 16 + usable`, ALWAYS.** ✅ **The axis already fills the window at every zoom level.**

⚠️ **THEREFORE THE SYMPTOM *"the timeline spans one dot"* IS NOT THE AXIS COLLAPSING.** ✅ **The axis is
full width; ⚠️ it is the DOTS that all merge into ONE AGGREGATE sitting on a full-width line.**
✅ **R1 needs NO structural change** — ⚠️ **only a guard that the derived span is never degenerate (§4.4).**

### 2.2 ⛔ R2 AND R3 ARE NOT IMPLEMENTED AT ALL

✅ **Today's algorithm is PROXIMITY CHAINING with a FIXED threshold** (`:1424`, `mergeThreshold :1417`):

```swift
if ox - clusterMaxX <= mergeThreshold(currentSize: members.count) {
    members.append(...); clusterMaxX = ox          // ⚠️ the edge ADVANCES
}
private func mergeThreshold(currentSize: Int) -> CGFloat {
    let r = currentSize >= 2 ? aggregateDotRadius : dotRadius
    return r + dotRadius + 2                        // ⚠️ CAPS at 18 px
}
```

⚠️ **TWO PROPERTIES MAKE R2 UNREACHABLE:**
⚠️ **(a) TRANSITIVITY.** ✅ **`clusterMaxX` advances to each new member, so A–B–C chains indefinitely.**
⚠️ **Membership does NOT mean "co-located"; it means "reachable by 18 px hops".**
⚠️ **(b) NO COUNT AWARENESS.** ✅ **The threshold caps at `11 + 5 + 2 = 18 px` regardless of cluster size
or of how many clusters result.** ⚠️ **Output count is an ACCIDENT of spacing.**

✅ **THE ARITHMETIC OF THE REPORTED CASE:** ⚠️ **1,158 items across `usable ≈ 1,000 px` ⇒ mean spacing
`< 1 px` ⇒ every hop is under 18 px ⇒ ONE cluster of 1,158.**

⚠️ **AND R3 IS NOT MERELY ABSENT — IT IS VIOLATED.** ⚠️ **Because threshold is fixed in PIXELS while
spacing scales with zoom, the transition is a CLIFF: ✅ a zoom step can take a chain from "all merged"
to "all separate" with nothing in between.**

---

## 3. ✅ The design

### 3.1 ✅ Replace proximity chaining with CAPACITY-BASED BUCKETING

⚠️ **The question stops being *"is this dot near that one?"* and becomes *"how many aggregates FIT?"***

```
capacity = floor(usable / aggregatePitch)        // aggregatePitch = 2*aggregateDotRadius + gap
```

✅ **`capacity` is the MAXIMUM number of distinct aggregates the width can show without overlap.**
✅ **Then bucket by POSITION, not by neighbour distance:**

```
bucketIndex(item) = clamp(floor((itemX - 16) / aggregatePitch), 0, capacity - 1)
```

✅ **Items sharing a bucket form one cluster; ⚠️ empty buckets produce NO cluster.**
✅ **A bucket holding exactly one item renders as a SINGLE DOT (existing `members.count == 1` path).**

⚠️ **WHY THIS SATISFIES R2:** ✅ **the number of clusters is bounded by `capacity` BY CONSTRUCTION, and
approaches it whenever items are spread** — ⚠️ **never one-when-many-would-fit.**
⚠️ **WHY IT SATISFIES R3:** ✅ **zooming in raises `visibleSpanMs`'s resolution, so items redistribute
across MORE buckets — ✅ strictly more clusters, strictly smaller, MONOTONIC.** ⚠️ **No cliff, because
bucket boundaries move continuously with zoom.**

⚠️ **TRANSITIVITY IS GONE:** ✅ **membership is now absolute (which bucket) rather than relational
(reachable by hops).**

### 3.1a ✅ **SUPERSEDED — buckets partition the STORY, not the viewport** (user ruling 2026-09-14)

⚠️ **§3.1's screen-space bucketing WAS IMPLEMENTED AND FALSIFIED ON THE RIG.** ✅ **It fixed the
count (R2) but made buckets STATIONARY SCREEN SLOTS** — ⚠️ **the dots flowed THROUGH them as the
user panned.** ✅ **The user's words:** *"The buckets stay clamped in place and the dots flow through
them… the buckets represent a specific cluster of dots, not just 'the dots under the timeline at this
point in time.'"*

✅ **THE RULED MODEL:**
✅ **(a) Buckets are EQUAL STORY-TIME SLICES across the WHOLE story.**
✅ **(b) ZOOM decides how many slices exist and where they sit; that assignment is then FIXED.**
✅ **(c) PAN slides the viewport across that stable layout — it NEVER re-forms the buckets.**

✅ **THE MECHANISM: the partition is computed in STORY-TIME space** (`offsetMs − minOffsetMs`,
divided by `sliceMs`), ⚠️ **so it is INDEPENDENT OF SCROLL BY CONSTRUCTION** — ✅ **`buildClusters`
reads NOTHING from `scrollOffsetFraction` when forming buckets; scroll only filters which slices are
DRAWN.**

⚠️ **EQUAL TIME SLICES, NOT EQUAL SCENE COUNTS** (⚠️ **the user's first phrasing said "first n
scenes, second n"; ✅ ruled to time slices on the tradeoff below**): ✅ **dots are positioned by
`offsetMs`, so a time-sliced bucket ALWAYS sits where its members actually are.** ⚠️ **Counts
therefore VARY — a dense month yields a fat bucket, a quiet year a thin one.** ⚠️ **Equal-COUNT
partitioning would decouple a bucket's screen position from its members' story time, which breaks
down precisely once dots are MANUALLY PLACED.**

✅ **ANCHORING: a multi-member cluster draws at its SLICE CENTRE, not at `members[0]`** — ⚠️ **so it
holds position as membership varies.** ✅ **A single-member cluster draws at its own member's `x`, so
a lone dot sits exactly on its scene's story time.** ⚠️ **`aggregateJoinOffsetMs` (drag-to-snap) was
updated to hit-test the ANCHOR** — ✅ **testing `members[0]` would snap to a point the dot no longer
occupies.**

⚠️ **EMPTY SLICES DRAW NOTHING** — ✅ **the timeline shows only where story actually is.**

### 3.2 ⚠️ The honest tradeoff — ✅ state it, do not hide it

⚠️ **BUCKETING CAN SPLIT A GENUINELY CO-LOCATED PAIR ACROSS A BOUNDARY.** ⚠️ **Two scenes 1 px apart
straddling a bucket edge render as two adjacent aggregates rather than one.**
✅ **THIS IS ACCEPTABLE AND IS THE RIGHT TRADE:** ⚠️ **the failure is TWO DOTS TOUCHING — ✅ visually
noisy but truthful, ⚠️ versus today's failure, which is ONE DOT FOR THE WHOLE STORY plus an
undismissable 6,600 pt panel.**
✅ **A refinement exists if the user wants it (⚠️ NOT proposed for v1): snap bucket edges to the widest
local gap.** ⚠️ **It costs a scan per layout and should not be built before the simple form is seen.**

### 3.3 ✅ The popover must be bounded INDEPENDENTLY

⚠️ **[I-0208] defect (2) IS NOT FIXED BY §3.1 AND MUST NOT BE ASSUMED TO BE.**
✅ **`AggregateMembersPopover` (`:1928-1942`) computes `ringR = max(spacing, count * spacing / 2π)`**
— ⚠️ **UNBOUNDED in `count`.**

⚠️ **§3.1 makes a 1,158-member cluster unlikely; ⛔ it does NOT make it impossible** — ✅ **1,158 scenes
sharing ONE story-time offset legitimately land in one bucket at ANY zoom.**
✅ **REQUIRED: cap the ring radius, and page or scroll members beyond a fixed count.**
⚠️ **A popover must never exceed its window.**

### 3.4 ✅ Sequencing

| step | work | ⚠️ note |
| ---- | ---- | ------- |
| **1** | ✅ **MEASURE** — log cluster count + max member count at several zooms on `dumas-prose` | ⚠️ **[I-0208] requires this; ✅ three code-read diagnoses were already wrong in this investigation** |
| **2** | ✅ **Bound the popover (§3.3)** | ✅ **INDEPENDENT; ✅ fixes the undismissable panel even if §3.1 slips** |
| **3** | ✅ **Capacity bucketing (§3.1)** | ⚠️ **the R2/R3 fix** |
| **4** | ✅ **Assert R3 in a test** | ✅ **zoom in ⇒ cluster count NON-DECREASING, max member count NON-INCREASING** |

⚠️ **STEP 2 BEFORE STEP 3, DELIBERATELY:** ✅ **the unreachable panel is the part that traps the user,
⚠️ and it is the smaller change.**

---

## 4. ⛔ Open questions — ✅ for the user, not assumed

- **Q1** ⚠️ **Should a bucket holding 2 items render as an AGGREGATE or as two small dots if they fit?**
  ✅ **Today `count == 1` ⇒ single dot.** ⚠️ **A bucket is `aggregatePitch` wide, so two regular dots
  MAY fit.** ⛔ **Not ruled.**
- **Q2** ⚠️ **`aggregatePitch` gap value** — ✅ **what visual separation between adjacent aggregates?**
  ⚠️ **Today's implied spacing is `18 px` total. ⛔ Not ruled.**
- **Q3** ⚠️ **Does R2 apply to the IMPORTED timeline rows too?** ✅ **They have their own aggregate path
  (`:859`).** ⚠️ **Consistency argues yes; ⛔ the user specified the MAIN row.**
- **Q4** ⚠️ **Popover member cap before paging (§3.3)?** ⛔ **Not ruled.**
- **Q5** ⚠️ **R1's degenerate guard: what if ALL scenes share ONE offset (`spanMs` → 1)?** ✅ **The axis
  still fills the width, but every dot lands at `x = 16`.** ⚠️ **Is that correct, or should a
  zero-span story get a synthetic minimum span?** ⛔ **Not ruled.**

---

## 5. ⛔ Not established

- ⛔ **The measured cluster/member counts at each zoom** (§3.4 step 1). ⚠️ **All arithmetic here is
  DERIVED FROM THE CODE, not observed.**
- ⛔ **Whether `requiredClusterHeight` (`:706`, the auto-grow of panel height for ring stacks)
  interacts with §3.1.** ⚠️ **It is driven by cluster size and MUST be re-checked.**
- ⛔ **Linux/Qt parity.** ⚠️ **The Linux timeline is a separate implementation; ✅ this doc is `[Apple]`.**
