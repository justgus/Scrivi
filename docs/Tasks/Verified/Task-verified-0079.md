---
name: T-0079
title: "Divider rendering — thin horizontal rule (1pt, system separator color) with no text, no labels"
sprint: SP-023
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Implemented as `DividerAttachmentCell: NSTextAttachmentCell` inside `ManuscriptTextView.swift`. Renders a 1pt horizontal rule using `NSBezierPath` with `NSColor.separatorColor`. Occupies 24pt of vertical space. No text, no labels — purely visual scene separation. Inserted between segments via `NSTextAttachment` in `rebuildStorage`.
