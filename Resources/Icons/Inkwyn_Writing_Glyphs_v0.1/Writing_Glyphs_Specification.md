# Inkwyn — Runic Whimsy Writing Glyph Family

Version: 0.1 — vector/raster implementation **for review** (six glyph concepts approved; assets not yet approved).

## Common design requirements

- All glyphs are monochrome, use `currentColor` in SVG, and have transparent PNG backgrounds.
- Three optical variants: Large, Medium, Small. PNG exports 512 × 512, with variants scaled by the UI.
- Silhouette and meaning take priority over runic flourish; no submenu indicators.
- Designs trace the selected concept board, retaining its actual geometry. Vector traces should be evaluated at target UI sizes before production approval.

## Glyph specifications

### Writing
**Recognition anchor:** Quill and inkwell.
**Construction and meaning:** An angled quill with flowing feather vanes emerges from an inkpot; a small runic diamond marks the inkpot. This represents the Writing workspace, not a manuscript object.
**Optical reduction:** Preserve the pointed quill, feather silhouette, curled flourish, and inkwell; reduce feather cuts in Medium and Small.
**Deliverables:** SVG + transparent PNG in Large/Medium/Small.

### History
**Recognition anchor:** Hourglass within a ceremonial scroll.
**Construction and meaning:** A vertically arranged scroll encloses an hourglass, joining historical record and the passage of time. The scroll ends have ornamental terminals.
**Optical reduction:** Keep both the hourglass silhouette and the top/bottom scroll rollers distinct; reduce terminal diamonds before losing the hourglass.
**Deliverables:** SVG + transparent PNG in Large/Medium/Small.

### Outline
**Recognition anchor:** Branching three-node hierarchy.
**Construction and meaning:** An upper diamond-linked root flows through paired organic branches to three lower diamond nodes, establishing an unmistakable hierarchy.
**Optical reduction:** Keep one parent and three children visually separate. Strengthen connecting strokes for Small.
**Deliverables:** SVG + transparent PNG in Large/Medium/Small.

### Sources
**Recognition anchor:** Book stack with visible bookmark.
**Construction and meaning:** An angled bound reference volume lies over another volume; a projecting ribbon bookmark is unmistakable at the front. A small runic star adorns the upper cover.
**Optical reduction:** The projecting bookmark is essential. Simplify cover ornament before simplifying the bookmark or stacked book profile.
**Deliverables:** SVG + transparent PNG in Large/Medium/Small.

### Tags
**Recognition anchor:** Hanging label with attachment loop.
**Construction and meaning:** An angled tag with an open attachment hole and loose looping cord. A four-point runic star occupies its face.
**Optical reduction:** Keep the dangling tag and upper attachment loop readable; omit internal star at very small display sizes only if needed.
**Deliverables:** SVG + transparent PNG in Large/Medium/Small.

### Todo
**Recognition anchor:** Framed checkbox with bold check.
**Construction and meaning:** A softly curved square or page-like frame contains an energetic bold checkstroke and a small runic diamond on its upper edge.
**Optical reduction:** Maintain clear separation between checkstroke and frame, especially at Small.
**Deliverables:** SVG + transparent PNG in Large/Medium/Small.

## Review checklist

- Does the Small History icon retain an actual hourglass inside the scroll?
- Is the Sources bookmark immediately identifiable at Small?
- Do the three diamond nodes in Outline remain separated?
- Is the Writing quill legible against the inkpot at toolbar scale?
- Can Tags and Todo be distinguished at Small?
- Do all six remain legible in both light and dark interface themes?

## Approval status

Concepts selected; generated optical-size assets pending user approval.